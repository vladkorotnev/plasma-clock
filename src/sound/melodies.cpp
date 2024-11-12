#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <service/localize.h>
#include <bits/stl_algobase.h>
#include <sound/pomf.h>
#include <service/disk.h>
#include <vector>
#include <dirent.h>
#include <utils.h>

static char LOG_TAG[] = "MUDB";

static const melody_item_t just_beep_data[] = {
    {FREQ_SET, 0, 1000},
    {DELAY, 0, 250},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 500},
};

static const melody_item_t pc98_pipo_data[] = {
    {FREQ_SET, 0, 2000},
    {DELAY, 0, 125},
    {FREQ_SET, 0, 1000},
    {DELAY, 0, 125},
};

StaticMelodySequence pc98_pipo(pc98_pipo_data, sizeof(pc98_pipo_data)/sizeof(melody_item_t), "PC-98 PiPo");

static const melody_item_t tulula_fvu_data[] = {
    {FREQ_SET, 0, 392},
    {DELAY, 0, 120},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 5},
    {FREQ_SET, 0, 392},
    {DELAY, 0, 120},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 5},
    {FREQ_SET, 0, 392},
    {DELAY, 0, 120},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 5},
    {FREQ_SET, 0, 293},
    {DELAY, 0, 250},    
};

StaticMelodySequence tulula_fvu(tulula_fvu_data, sizeof(tulula_fvu_data)/sizeof(melody_item_t), "Русь-28");

static const melody_item_t oelutz_fvu_data[] = {
    {FREQ_SET, 0, 587},
    {DELAY, 0, 250},
    {FREQ_SET, 0, 698},
    {DELAY, 0, 250},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 250},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 125},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 500},
};

StaticMelodySequence oelutz_fvu(oelutz_fvu_data, sizeof(oelutz_fvu_data)/sizeof(melody_item_t), "Штрих-М");

std::vector<MelodySequence *> all_chime_list = {
    new StaticMelodySequence(just_beep_data, sizeof(just_beep_data)/sizeof(melody_item_t), "Beep"),
    &pc98_pipo,
    &tulula_fvu,
    &oelutz_fvu
};

int all_chime_count = all_chime_list.size();

MelodySequence * melody_from_no(int melody_no) {
    if(melody_no >= all_chime_list.size()) melody_no = 0;
    return all_chime_list[melody_no];
}


class PomfMelodySequence: public MelodySequence {
public:
    PomfMelodySequence(const char * sourcePath) {
        strncpy(path, sourcePath, 63);
        try_preload_metadata();
    }

    ~PomfMelodySequence() {
        unload();
        if(title_ != nullptr) {
            free(title_); title_ = nullptr;
        }
        if(array_ != nullptr) {
            free(array_); array_ = nullptr;
        }
    }

    bool valid() { return exists; }
    bool load() override { 
        if(array_ != nullptr) {
            free(array_);
            array_ = nullptr;
        }
        unload_samples();

        num_rows_ = 0;

        ESP_LOGI(TAG, "Load %s", path);
        FILE * f = nullptr;
        POMFHeader head = { 0 };
        POMFChunkHeader chunk = { 0 };
        rle_sample_t * cur_samp = nullptr;
        size_t r = 0;
        size_t total = 0;
        bool reading = true;

        f = fopen(path, "rb");
        if(!f) {
            ESP_LOGE(TAG, "File open error: %s (%i, %s)", path, errno, strerror(errno));
            goto bail;
        }

        fseek(f, 0, SEEK_SET);
        r = fread(&head, 1, sizeof(POMFHeader), f); //<- opposite order of size and count isn't working, esp stdlib buggy?
        if(r != sizeof(POMFHeader)) {
            ESP_LOGE(TAG, "Header read error, wanted %i bytes, got %i", sizeof(POMFHeader), r);
            goto bail;
        }

        if(head.magic != POMF_MAGIC_FILE) {
            ESP_LOGE(TAG, "MAGIC not match, expected 0x%x, got 0x%x", POMF_MAGIC_FILE, head.magic);
            goto bail;
        }

        if(head.version != POMF_CURVER) {
            ESP_LOGE(TAG, "Version not match, expected 0x%x, got 0x%x", POMF_CURVER, head.version);
            goto bail;
        }

        exists = true;

        do {
            total = 0;
            while(total < sizeof(POMFChunkHeader) && r > 0) {
                r = fread(((void*)&chunk) + total, 1, sizeof(POMFChunkHeader) - total, f);
                total += r;
            }
            if(total != sizeof(POMFChunkHeader)) {
                ESP_LOGE(TAG, "ChunkHead read error, wanted %i bytes, got %i", sizeof(POMFChunkHeader), r);
                goto bail;
            }

            switch(chunk.magic) {
                case POMF_MAGIC_SAMPLE:
                case POMF_MAGIC_SAMPLE_COMPRESSED:
                    cur_samp = (rle_sample_t*) malloc(sizeof(rle_sample_t));
                    if(cur_samp == nullptr) {
                        ESP_LOGE(TAG, "OOM allocating sample header");
                        reading = false;
                        goto bail;
                    } else {
                        cur_samp = (rle_sample_t*) malloc(chunk.size);
                        if(cur_samp == nullptr) {
                            ESP_LOGE(TAG, "OOM allocating sample (wanted %i bytes)", chunk.size);
                            reading = false;
                            goto bail;
                        }
                        total = 0;
                        while(total < chunk.size && r > 0) {
                            r = fread(((void*)cur_samp) + total, 1, chunk.size - total, f);
                            total += r;
                        }
                        if(total != chunk.size) {
                            ESP_LOGE(TAG, "saMP expected %i bytes, got %i", chunk.size, total);
                            reading = false;
                            goto bail;
                        } else {
                            ESP_LOGV(TAG, "saMP read %i bytes", total);
                            if(chunk.magic == POMF_MAGIC_SAMPLE_COMPRESSED) {
                                cur_samp = (rle_sample_t*) decompress_emplace(cur_samp, chunk.size, chunk.realsize);
                                if(cur_samp == nullptr) {
                                    reading = false;
                                    goto bail;
                                }
                                ESP_LOGV(TAG, "saZZ decompressed to %i bytes", chunk.realsize);
                            }
                            // Fix the data pointer to point after the header
                            cur_samp->rle_data = (const uint8_t *) &cur_samp[1];
                            ESP_LOGI(TAG, "saMP @ 0x%x: SR=%i, root=%i, len=%i, mode=%i, data=0x%x", cur_samp, cur_samp->sample_rate, cur_samp->root_frequency, cur_samp->length, cur_samp->mode, cur_samp->rle_data);
                            samples.push_back(cur_samp);
                            cur_samp = nullptr;
                        }
                    }
                    break;
                case POMF_MAGIC_TRACK:
                case POMF_MAGIC_TRACK_COMPRESSED:
                    if(array_ != nullptr) {
                        ESP_LOGE(TAG, "Multiple tuNE chunks, wtf!!");
                        reading = false;
                        goto bail;
                    } else {
                        array_ = (melody_item_t*) malloc(chunk.size);
                        if(array_ == nullptr) {
                            ESP_LOGE(TAG, "OOM allocating track");
                            reading = false;
                            goto bail;
                        }
                        total = 0;
                        while(total < chunk.size && r > 0) {
                            r = fread(((void*)array_) + total, 1, chunk.size - total, f);
                            total += r;
                        }
                        if(total != chunk.size) {
                            ESP_LOGE(TAG, "tuNE expected %i bytes, got %i", chunk.size, total);
                            reading = false;
                            goto bail;
                        } else {
                            if(chunk.magic == POMF_MAGIC_TRACK_COMPRESSED) {
                                array_ = (melody_item_t*) decompress_emplace((void*) array_, chunk.size, chunk.realsize);
                                if(array_ == nullptr) {
                                    reading = false;
                                    goto bail;
                                } else {
                                    total = chunk.realsize;
                                    ESP_LOGV(TAG, "tuZZ decompressed to %i bytes", chunk.realsize);
                                }
                            }
                            num_rows_ = total / sizeof(melody_item_t);
                            ESP_LOGV(TAG, "tuNE read %i rows in %i bytes", num_rows_, total);
                            for(int i = 0; i < num_rows_; i++) {
                                // Fix up sample pointers
                                if(array_[i].command == SAMPLE_LOAD) {
                                    int sample_num = array_[i].argument;
                                    if(sample_num >= samples.size()) {
                                        ESP_LOGE(TAG, "Sample number %i out of bounds! (have %i)", sample_num, samples.size());
                                        reading = false;
                                        goto bail;
                                    } else {
                                        array_[i].argument = (int) samples[sample_num];
                                    }
                                }
                            }
                        }
                    }
                    break;
                case POMF_MAGIC_END:
                    if(array_ != nullptr) {
                        ESP_LOGV(TAG, "Found EOF, success");
                    } else {
                        ESP_LOGE(TAG, "File without tuNE!!");
                    }
                    reading = false;
                    break;
                default:
                    ESP_LOGE(TAG, "Unknown chunk type %x abort!!", chunk.magic);
                    reading = false;
                    goto bail;
                    break;
            }
        } while(reading);

        loaded = true;
        fclose(f);
        return true;

    bail:
        exists = false;
        if(f != nullptr) fclose(f);
        if(array_ != nullptr) free(array_);
        array_ = nullptr;
        if(cur_samp != nullptr) free(cur_samp);
        unload_samples();
        num_rows_ = 0;
        return false;
    }
    void unload() override {
        ESP_LOGI(TAG, "Unload %s", path);
        if(array_ != nullptr) {
            free(array_); array_ = nullptr;
        }
        num_rows_ = 0;
        loaded = false;
        unload_samples();
    }
    const char * get_title() override { return title_; }
    const char * get_long_title() override { return long_title_; }
    const melody_item_t * get_array() override { return array_; }
    int get_num_rows() override { return num_rows_; }

protected:
    static constexpr const char * TAG = "POMF";
    bool exists = false;
    bool loaded = false;
    char path[64] = { 0 };
    char * title_ = nullptr;
    char * long_title_ = nullptr;
    melody_item_t * array_ = nullptr;
    std::vector<rle_sample_t *> samples = {};
    int num_rows_ = 0;

    void unload_samples() {
        for(auto i: samples) {
            free(i);
        }
        samples.clear();
    }

    void try_preload_metadata() {
        if(title_ != nullptr) {
            free(title_);
            title_ = nullptr;
        }

        if(long_title_ != nullptr) {
            free(long_title_);
            long_title_ = nullptr;
        }
        ESP_LOGV(TAG, "Prefetch %s", path);
        FILE * f = nullptr;
        POMFHeader head = { 0 };
        size_t r = 0;

        f = fopen(path, "rb");
        if(!f) {
            ESP_LOGE(TAG, "File open error: %s (%i, %s)", path, errno, strerror(errno));
            goto bail;
        }

        fseek(f, 0, SEEK_SET);
        r = fread(&head, 1, sizeof(POMFHeader), f); //<- opposite order of size and count isn't working, esp stdlib buggy?
        if(r != sizeof(POMFHeader)) {
            ESP_LOGE(TAG, "Header read error, wanted %i bytes, got %i", sizeof(POMFHeader), r);
            goto bail;
        }

        if(head.magic != POMF_MAGIC_FILE) {
            ESP_LOGE(TAG, "MAGIC not match, expected 0x%x, got 0x%x", POMF_MAGIC_FILE, head.magic);
            goto bail;
        }

        if(head.version != POMF_CURVER) {
            ESP_LOGE(TAG, "Version not match, expected 0x%x, got 0x%x", POMF_CURVER, head.version);
            goto bail;
        }

        title_ = (char*) malloc(strlen(head.short_title));
        strcpy(title_, head.short_title);
        long_title_ = (char*) malloc(strlen(head.long_title));
        strcpy(long_title_, head.long_title);

        exists = true;

        fclose(f);
        return;

    bail:
        exists = false;
        if(f != nullptr) fclose(f);
    }
};

class RandomMelodySequence: public MelodySequence {
public:
    RandomMelodySequence(const std::vector<MelodySequence*> & list, size_t offs):
        melodies { list },
        offset { offs } {}

    bool load() { 
        int no = 0;
        static int last_random_no = -1;
        do {
            no = offset + (esp_random() % (all_chime_count - offset));
        } while(no == last_random_no);
        last_random_no = no;

        choice = melodies[no];
        ESP_LOGI(LOG_TAG, "Random choice %i: %s", no, choice->get_title());
        if(choice != nullptr)
            return choice->load();

        return false;
    }

    void unload() {
        if(choice != nullptr) {
            choice->unload();
            choice = nullptr;
        }
    }

    const char * get_title() { return localized_string("(Randomize)"); }
    const char * get_long_title() { return get_title(); }
    const melody_item_t * get_array() {
        if(choice != nullptr) return choice->get_array();
        return nullptr; 
    }
    int get_num_rows() { 
        if(choice != nullptr) return choice->get_num_rows();
        return 0; 
    }
protected:
    const std::vector<MelodySequence*>& melodies;
    MelodySequence * choice = nullptr;
    size_t offset;
};

#define MUSIC_DIR (FS_MOUNTPOINT "/music")
void load_melodies_from_disk() {
    DIR *d;
    struct dirent *dir;
    d = opendir(MUSIC_DIR);
    char pbuf[64] = { 0 };
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if(dir->d_type == DT_REG && strlen(dir->d_name) > 5 && !strcmp(dir->d_name + strlen(dir->d_name) - 5, ".pomf")) {
                ESP_LOGV(LOG_TAG, "Found music file: %s", dir->d_name);
                snprintf(pbuf, 63, "%s/%s", MUSIC_DIR, dir->d_name);
                auto seq = new PomfMelodySequence(pbuf);
                if(seq->valid()) {
                    ESP_LOGV(LOG_TAG, "Title: %s", seq->get_title());
                    all_chime_list.push_back(seq);
                } else {
                    ESP_LOGE(LOG_TAG, "File %s not valid", dir->d_name);
                    delete seq;
                }
                taskYIELD();
            }
        }
        closedir(d);
    }
    all_chime_count = all_chime_list.size();
    all_chime_list.insert(all_chime_list.begin(), new RandomMelodySequence(all_chime_list, 5));

    // TODO: Get rid of those two vectors entirely
    all_chime_names.clear();
    for(int i = 0; i < all_chime_list.size(); i++) {
        all_chime_names.push_back(all_chime_list[i]->get_title());
    }
}

std::vector<const char *> all_chime_names = {
    "Beep",
    "PC-98 Boot",
    "Русь 28",
    "Штрих-M",
};