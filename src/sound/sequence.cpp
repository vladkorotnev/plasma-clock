#include <sound/sequence.h>
#include <sound/pomf.h>
#include <esp32-hal-log.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <utils.h>

static constexpr const char TAG[] = "POMF";

PomfMelodySequence::PomfMelodySequence(const char * sourcePath) {
    strncpy(path, sourcePath, 63);
    try_preload_metadata();
}

PomfMelodySequence::~PomfMelodySequence() {
    unload();
    if(title_ != nullptr) {
        free(title_); title_ = nullptr;
    }
    if(array_ != nullptr) {
        free(array_); array_ = nullptr;
    }
}

bool PomfMelodySequence::load() { 
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
void PomfMelodySequence::unload() {
    ESP_LOGI(TAG, "Unload %s", path);
    if(array_ != nullptr) {
        free(array_); array_ = nullptr;
    }
    num_rows_ = 0;
    loaded = false;
    unload_samples();
}

void PomfMelodySequence::unload_samples() {
    for(auto i: samples) {
        free(i);
    }
    samples.clear();
}

void PomfMelodySequence::try_preload_metadata() {
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
