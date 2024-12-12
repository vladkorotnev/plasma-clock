#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <service/localize.h>
#include <bits/stl_algobase.h>
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