#include <sound/pomf.h>

extern "C" const POMFHeader POMF_HEAD = {
    POMF_MAGIC_FILE,
    POMF_CURVER,
    "Shabon",
    "Hachiya Nanashi - Shabon"
};

extern "C" const melody_item_t POMF_TUNE[] = {
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 659},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 659},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 635},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 635},
    {DELAY, 0, 254},
    /* HOOK */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_HOOK_START},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1397},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1397},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 381},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 508},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 254},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 169},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 169},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 169},
    {FREQ_SET, 0, 988},
    {DELAY, 0, 169},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 169},
    {FREQ_SET, 0, 784},
    {DELAY, 0, 169},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 3322},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 3520},
    {DELAY, 0, 126},
    {FREQ_SET, 0, 3520},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 2637},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 2349},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 2489},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 2093},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 1760},
    {DELAY, 0, 127},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 2033},
};