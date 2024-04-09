#pragma once
#include <sound/sequencer.h>

extern const melody_sequence_t just_beep;
extern const melody_sequence_t pc98_pipo;
extern const melody_sequence_t tulula_fvu;
extern const melody_sequence_t oelutz_fvu;
extern const melody_sequence_t arise;
extern const melody_sequence_t caramell;
extern const melody_sequence_t duvet;
extern const melody_sequence_t truth;
extern const melody_sequence_t haruhi_no_omoi;

static constexpr const char * all_chime_names = "Just Beep,PC-98 Boot Chime,Russ28 Notification Sound,Shtrikh-M POS Boot Chime,A.M. - Arise,Caramelldansen,BoA - Duvet,T-Square - Truth,Haruhi no Omoi,Random";
static const melody_sequence_t all_chime_list[] = {
    just_beep,
    pc98_pipo,
    tulula_fvu,
    oelutz_fvu,
    arise,
    caramell,
    duvet,
    truth,
    haruhi_no_omoi
};
static const int all_chime_count = sizeof(all_chime_list)/sizeof(melody_sequence_t);