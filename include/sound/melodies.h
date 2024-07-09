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
extern const melody_sequence_t wpip;
extern const melody_sequence_t hishoku;
extern const melody_sequence_t bouken;
extern const melody_sequence_t gentle_jena;
extern const melody_sequence_t gammapolisz;
extern const melody_sequence_t like_the_wind;

static constexpr const char * all_chime_names = "Just Beep,PC-98 Boot Chime,Russ28 Notification Sound,Shtrikh-M POS Boot Chime,A.M. - Arise,Caramelldansen,BoA - Duvet,T-Square - Truth,Haruhi no Omoi,When Present Is Past,Hishoku no Sora,Bouken Desho Desho,Gentle Jena,Gammapolisz,Like The Wind (TMMS),Random";
static const melody_sequence_t all_chime_list[] = {
    just_beep,
    pc98_pipo,
    tulula_fvu,
    oelutz_fvu,
    arise,
    caramell,
    duvet,
    truth,
    haruhi_no_omoi,
    wpip,
    hishoku,
    bouken,
    gentle_jena,
    gammapolisz,
    like_the_wind,
};
static const int all_chime_count = sizeof(all_chime_list)/sizeof(melody_sequence_t);