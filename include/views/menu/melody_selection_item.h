#pragma once
#include "list_selection_item.h"
#include <sound/melodies.h>

class MenuMelodySelectorView: public MenuListSelectorView {
public:
    MenuMelodySelectorView(NewSequencer * preview, const char * title, int melodyNo, std::function<void(bool, Renderable*)> onActivated, std::function<void(int)> onChange):
        MenuListSelectorView(
            title,
            all_chime_names,
            melodyNo,
            [this, onActivated](bool active, Renderable* instance) {
                if(!active && sequencer) sequencer->stop_sequence();

                onActivated(active, instance);
            },
            [this, onChange](int newMelodyNo) {
                sequencer->stop_sequence();
                if(newMelodyNo != all_chime_count) // Don't play the random one
                    sequencer->play_sequence(melody_from_no(newMelodyNo), SEQUENCER_PLAY_HOOK_ONLY);
                onChange(newMelodyNo);
            } 
        ) {
            sequencer = preview;
        }

    
private:
    NewSequencer * sequencer;
};

class MenuMelodySelectorPreferenceView: public MenuMelodySelectorView {
public:
    MenuMelodySelectorPreferenceView(NewSequencer * preview, const char * title, prefs_key_t prefs_key, std::function<void(bool, Renderable*)> onActivated):
        key(prefs_key),
        MenuMelodySelectorView(preview, title, prefs_get_int(prefs_key), onActivated, [this](int newVal) { prefs_set_int(key, newVal); }) {}

    void step() {
        currentValue = prefs_get_int(key);
        MenuMelodySelectorView::step();
    }

private:
    prefs_key_t key;
};