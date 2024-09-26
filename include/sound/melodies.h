#pragma once
#include <sound/sequencer.h>
#include <vector>

extern StaticMelodySequence pc98_pipo;
extern StaticMelodySequence tulula_fvu;
extern StaticMelodySequence oelutz_fvu;
extern std::vector<MelodySequence *> all_chime_list;
extern std::vector<const char *> all_chime_names;
extern int all_chime_count;
MelodySequence * melody_from_no(int melody_no);
void load_melodies_from_disk();
