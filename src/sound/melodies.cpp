#include <sound/sequencer.h>
#include <sound/melodies.h>

static const melody_item_t pc98_pipo_data[] = {
    {2000, 125},
    {1000, 125}
};
const melody_sequence_t pc98_pipo = {
    .array = pc98_pipo_data,
    .count = sizeof(pc98_pipo_data)/sizeof(melody_item_t)
};

static const melody_item_t tulula_fvu_data[] = {
    {392, 125},
    {392, 125},
    {392, 125},
    {293, 250}
};
const melody_sequence_t tulula_fvu = {
    .array = tulula_fvu_data,
    .count = sizeof(tulula_fvu_data)/sizeof(melody_item_t)
};

static const melody_item_t oelutz_fvu_data[] = {
    {587, 250},
    {698, 250},
    {880, 250},
    {1046, 125},
    {988, 500},
};
const melody_sequence_t oelutz_fvu = {
    .array = oelutz_fvu_data,
    .count = sizeof(oelutz_fvu_data)/sizeof(melody_item_t)
};