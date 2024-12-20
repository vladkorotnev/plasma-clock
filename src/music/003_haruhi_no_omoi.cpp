#include <sound/pomf.h>

extern "C" const POMFHeader POMF_HEAD = {
    POMF_MAGIC_FILE,
    POMF_CURVER,
    "Haruhi no omoi",
    "Kosaki Satoru - Haruhi no omoi"
};

extern "C" const melody_item_t POMF_TUNE[] = {
    {FREQ_SET, 0, 554},
    {DELAY, 0, 681},
    {FREQ_SET, 0, 740},
    {DELAY, 0, 1363},
    {FREQ_SET, 0, 698},
    {DELAY, 0, 681},
    {FREQ_SET, 0, 1108},
    {DELAY, 0, 1363},
    {FREQ_SET, 0, 1108},
    {DELAY, 0, 681},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 1363},
    {FREQ_SET, 0, 1108},
    {DELAY, 0, 681},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 1363},
    {FREQ_SET, 0, 932},
    {DELAY, 0, 340},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 340},
    {FREQ_SET, 0, 1108},
    {DELAY, 0, 1363},
    {FREQ_SET, 0, 740},
    {DELAY, 0, 681},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 1363},
    {FREQ_SET, 0, 932},
    {DELAY, 0, 681},
    {FREQ_SET, 0, 830},
    {DELAY, 0, 2045},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 3579},
};
