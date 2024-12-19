#include <sound/pomf.h>
extern "C" const POMFHeader POMF_HEAD = {
    POMF_MAGIC_FILE,
    POMF_CURVER,
    "MTC",
    "S3RL - MTC", //<- It won't make sense even if you hear this song :P
};

extern "C" const melody_item_t POMF_TUNE[] = {
    {FREQ_SET, 0, 466},
    {DUTY_SET, 0, 8},
    {DUTY_SET, 1, 6},
    {DUTY_SET, 2, 8},
    {DUTY_SET, 3, 8},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 428},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 428},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 415},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 428},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 428},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 415},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 2, 92},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 428},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 2, 87},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 415},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {FREQ_SET, 2, 104},
    {DELAY, 0, 257},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 428},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 2, 87},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 705},
    {FREQ_SET, 0, 698},
    {DELAY, 0, 322},
    {FREQ_SET, 2, 92},
    {DELAY, 0, 19},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 25},
    {FREQ_SET, 0, 466},
    {DELAY, 0, 34},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 48},
    {FREQ_SET, 0, 740},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 62},
    {FREQ_SET, 0, 698},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 170},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 322},
    {FREQ_SET, 2, 69},
    {DELAY, 0, 19},
    {FREQ_SET, 0, 415},
    {DELAY, 0, 25},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 34},
    {FREQ_SET, 0, 698},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 111},
    {FREQ_SET, 0, 554},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 322},
    {FREQ_SET, 2, 65},
    {DELAY, 0, 19},
    {FREQ_SET, 0, 311},
    {DELAY, 0, 25},
    {FREQ_SET, 0, 415},
    {DELAY, 0, 34},
    {FREQ_SET, 0, 523},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 625},
    {FREQ_SET, 0, 261},
    {DELAY, 0, 317},
    {FREQ_SET, 0, 277},
    {DELAY, 0, 5},
    {FREQ_SET, 2, 69},
    {DELAY, 0, 335},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 5},
    {FREQ_SET, 0, 261},
    {DELAY, 0, 299},
    {FREQ_SET, 0, 233},
    {DELAY, 0, 341},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 42},
    /* HOOK */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_HOOK_START},
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_LOOP},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 1, 830},
    {DELAY, 0, 342},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1864},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1244},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 415},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 415},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 698},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 698},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 85},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 1661},
    {DELAY, 0, 85},
    {FREQ_SET, 0, 932},
    {FREQ_SET, 1, 1864},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 85},
    {FREQ_SET, 0, 698},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 85},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 698},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 698},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 698},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 1244},
    {FREQ_SET, 2, 174},
    {FREQ_SET, 3, 87},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 523},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 523},
    {FREQ_SET, 1, 1046},
    {DELAY, 0, 171},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 932},
    {DELAY, 0, 514},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 342},
    /* HOOKEND */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_HOOK_END},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1864},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1244},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1244},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1046},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1046},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1046},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1864},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 92},
    {FREQ_SET, 3, 46},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 69},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1864},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 104},
    {FREQ_SET, 3, 52},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1244},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1864},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1661},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1864},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1661},
    {DELAY, 0, 85},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 116},
    {DELAY, 0, 171},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1864},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 185},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1244},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1244},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 277},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 155},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 155},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1046},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 155},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 155},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1046},
    {FREQ_SET, 2, 207},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 174},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1046},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 174},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 174},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1046},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 174},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 830},
    {FREQ_SET, 2, 233},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 932},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1108},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1864},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1480},
    {FREQ_SET, 2, 116},
    {FREQ_SET, 3, 185},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1480},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 138},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1108},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1661},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1864},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 1397},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 85},
    {FREQ_SET, 1, 1397},
    {DELAY, 0, 42},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 0, 1046},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 131},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 128},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 42},
    {FREQ_SET, 0, 1108},
    {FREQ_SET, 2, 155},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 932},
    {DELAY, 0, 342},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
};
