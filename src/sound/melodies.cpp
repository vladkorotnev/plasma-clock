#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <bits/stl_algobase.h>

const melody_sequence_t * melody_from_no(int melody_no) {
    int no = std::min(melody_no, all_chime_count);
    no  = std::max(no, 0);
    if(no == all_chime_count) {
        no = esp_random() % all_chime_count;
    }
    
    return &all_chime_list[no];
}

#define MELODY_OF(x) {\
    .array = x,\
    .count = sizeof(x)/sizeof(melody_item_t)\
}

static const melody_item_t just_beep_data[] = {
    {FREQ_SET, 0, 1000},
    {DELAY, 0, 250},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 500},
};
const melody_sequence_t just_beep = MELODY_OF(just_beep_data);

static const melody_item_t pc98_pipo_data[] = {
    {FREQ_SET, 0, 2000},
    {DELAY, 0, 125},
    {FREQ_SET, 0, 1000},
    {DELAY, 0, 125},
};
const melody_sequence_t pc98_pipo = MELODY_OF(pc98_pipo_data);

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
const melody_sequence_t tulula_fvu = MELODY_OF(tulula_fvu_data);

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
const melody_sequence_t oelutz_fvu = MELODY_OF(oelutz_fvu_data);

#include "melodies/am_arise.h"
#include "melodies/caramelldansen.h"
#include "melodies/duvet.h"
#include "melodies/truth.h"
#include "melodies/haruhi_no_omoi.h"
#include "melodies/when_present_is_past.h"
#include "melodies/hishoku_no_sora.h"
#include "melodies/bouken_desho_desho.h"
#include "melodies/gentle_jena.h"
#include "melodies/gammapolisz.h"
#include "melodies/like_the_wind.h"
#include "melodies/waiting_freqs.h"
#include "melodies/the_way.h"
#include "melodies/guitar_hero.h"
#include "melodies/shabon.h"
#include "melodies/steiner.h"
#include "melodies/gosenfu.h"
#include "melodies/mermaid_girl.h"
#include "melodies/eye_opener.h"
#include "melodies/ark.h"
#include "melodies/skibidi.h"
#include "melodies/kamippoina.h"
#include "melodies/re_sublimity.h"
#include "melodies/in_the_80s.h"
#include "melodies/shake.h"
#include "melodies/en_elmegyek.h"
#include "melodies/scatman.h"

const melody_sequence_t all_chime_list[] = {
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
    guitar_hero,
    syabon,
    steiner,
    towa,
    mermgirl,
    eye_opener,
    ark,
    skibidi,
    kamippoina,
    re_sublimity,
    inthe80s,
    shakeit,
    en_elmegyek,
    scatman,
};
const int all_chime_count = sizeof(all_chime_list)/sizeof(melody_sequence_t);

const char * all_chime_names_csv = "Just Beep,PC-98 Boot,Русь 28,Штрих-М,A.M. - Arise,Caramelldansen,BoA - Duvet,T-Square - Truth,Haruhi no Omoi,WAVE & DRAX - When Present Is Past,Hishoku no Sora,Bouken Desho Desho,Gentle Jena,Omega - Gammapolisz,Like The Wind (TMMS),NightRadio - Waiting Freqs,NightRadio - The Way,Dougal &amp; Gammer - Guitar Hero,Hachiya Nanashi - Shabon,Gate Of Steiner,deadballP - Towa ni tsuzuku gosenfu,Cream puff - Mermaid girl,Brisk &amp; Trixxy - Eye Opener,Hiroyuki Oshima - The Ark Awakes From The Sea Of Chaos,Skibidi Toilet (Timbaland - Give It To Me),PinocchioP - Kamippoina (God-ish),KOTOKO - Re-sublimity (Kannazuki no Miko),Calvin Harris - Acceptable In The 80s,Emon - Shake It,Omega - En Elmegyek,John Scatman - Scatman (Ski-Ba-Bop-Dop-Bop),Random";
const std::vector<const char *> all_chime_names = {
    "Beep",
    "PC-98 Boot",
    "Русь 28",
    "Штрих-M",
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
    "Guitar Hero",
    "Shabon",
    "Gate Of Steiner",
    "Towa ni tsuzuku gosenfu",
    "Mermaid girl",
    "Eye Opener",
    "The Ark Awakes From The Sea Of Chaos",
    "Skibidi Toilet / Give It To Me",
    "Kamippoina / God-ish",
    "Re-sublimity",
    "Acceptable In The 80s",
    "Shake It!",
    "En Elmegyek",
    "Scatman",
    "(Randomize)"
};