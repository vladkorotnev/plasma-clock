#include <sound/pomf.h>

extern "C" const POMFHeader POMF_HEAD = {
    POMF_MAGIC_FILE,
    POMF_CURVER,
    "Scar Tissue",
    "Red Hot Chili Peppers - Scar Tissue"
};

extern "C" const melody_item_t POMF_TUNE[] = {
    {DUTY_SET, 0, 5},
    {FREQ_SET, 0, 329},
    {DUTY_SET, 1, 6},
    {DUTY_SET, 2, 6},
    {DUTY_SET, 3, 8},
    {DELAY, 0, 674},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 247},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 247},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 277},
    {DELAY, 0, 505},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 277},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 277},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 277},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 659},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 247},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 674},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 337},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 740},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 370},
    {FREQ_SET, 1, 277},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 370},
    {FREQ_SET, 1, 277},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 247},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 277},
    {DELAY, 0, 1516},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 1, 277},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 3, 207},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {DELAY, 0, 505},
    /* LOOP */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_LOOP},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 92},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 123},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 277},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 277},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 277},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 78},
    {DELAY, 0, 28},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 1, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 3, 123},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 370},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 2, 370},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 370},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 2, 370},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 370},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 165},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    /* HOOK */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_HOOK_START},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 311},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 165},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 277},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 277},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 277},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 622},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 165},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 3, 123},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 329},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 988},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 740},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 740},
    {FREQ_SET, 1, 247},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 830},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 1318},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 1244},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 1108},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 168},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    /* HOOKEND */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_HOOK_END},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 370},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 2, 370},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 370},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 82},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 104},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {FREQ_SET, 2, 370},
    {FREQ_SET, 3, 110},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 622},
    {FREQ_SET, 1, 370},
    {FREQ_SET, 2, 494},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 554},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 138},
    {DELAY, 0, 168},
    {FREQ_SET, 0, 659},
    {FREQ_SET, 1, 415},
    {FREQ_SET, 2, 554},
    {FREQ_SET, 3, 165},
    {FREQ_SET, 4, 659},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 138},
    {FREQ_SET, 4, 1318},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 84},
    {FREQ_SET, 0, 466},
    {FREQ_SET, 1, 277},
    {DELAY, 0, 28},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 311},
    {FREQ_SET, 3, 123},
    {FREQ_SET, 4, 988},
    {DELAY, 0, 84},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 252},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 3, 0},
};
