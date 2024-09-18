#pragma once
#include <sound/sequencer.h>
#include <vector>

extern const melody_sequence_t just_beep;
extern const melody_sequence_t pc98_pipo;
extern const melody_sequence_t tulula_fvu;
extern const melody_sequence_t oelutz_fvu;

extern const char * all_chime_names_csv;
extern const std::vector<const char *> all_chime_names;
extern const melody_sequence_t all_chime_list[];
extern const int all_chime_count;
const melody_sequence_t * melody_from_no(int melody_no);