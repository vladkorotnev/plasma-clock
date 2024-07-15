#pragma once
#include <sound/sequencer.h>
#include <vector>

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
extern const melody_sequence_t waiting_freqs;
extern const melody_sequence_t the_way;

static constexpr const char * all_chime_names_csv = "Just Beep,PC-98 Boot,Русь 28,Штрих-М,A.M. - Arise,Caramelldansen,BoA - Duvet,T-Square - Truth,Haruhi no Omoi,When Present Is Past,Hishoku no Sora,Bouken Desho Desho,Gentle Jena,Gammapolisz,Like The Wind (TMMS),NightRadio - Waiting Freqs,NightRadio - The Way,Random";
static const std::vector<const char *> all_chime_names = {
    "Beep",
    "PC-98 Boot",
    "\x90\xE3\xE1\xEC 28",
    "\x98\xE2\xE0\xA8\xE5-M",
    "A.M. - Arise",
    "Caramelldansen",
    "BoA - Duvet",
    "T-Square - Truth",
    "Haruhi no Omoi",
    "When Present Is Past",
    "Hishoku no Sora",
    "Bouken Desho Desho",
    "Gentle Jena",
    "Gammapolisz",
    "Like The Wind (TMMS)",
    "Waiting Freqs",
    "The Way",
    "(Randomize)"
};
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
    waiting_freqs,
    the_way,
};
static const int all_chime_count = sizeof(all_chime_list)/sizeof(melody_sequence_t);

melody_sequence_t melody_from_no(int melody_no);