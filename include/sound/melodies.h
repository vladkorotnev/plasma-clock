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
extern const melody_sequence_t guitar_hero;
extern const melody_sequence_t syabon;
extern const melody_sequence_t steiner;
extern const melody_sequence_t towa;

extern const char * all_chime_names_csv;
extern const std::vector<const char *> all_chime_names;
extern const melody_sequence_t all_chime_list[];
extern const int all_chime_count;
melody_sequence_t melody_from_no(int melody_no);