#include <sound/pomf.h>

extern "C" const POMFHeader POMF_HEAD = {
    POMF_MAGIC_FILE,
    POMF_CURVER,
    "Give It To Me (Skibidi Toilet)",
    "Timbaland - Give It To Me (Skibidi Toilet)"
};

extern "C" const melody_item_t POMF_TUNE[] = {
    {DUTY_SET, 0, 3},
    {FREQ_SET, 0, 880},
    {DUTY_SET, 1, 5},
    {DUTY_SET, 2, 10},
    {DUTY_SET, 3, 8},
    {DELAY, 0, 160},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 880},
    {DELAY, 0, 240},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 160},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 1318},
    {DELAY, 0, 240},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 140},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 0, 1174},
    {DELAY, 0, 180},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 160},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 160},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 180},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 180},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 4, 1108},
    {DELAY, 0, 40},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 160},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 160},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 80},
    {FREQ_SET, 0, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 0, 1046},
    {DELAY, 0, 200},
    {FREQ_SET, 0, 0},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 80},
    /* LOOP */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_LOOP},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 200},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 293},
    {DELAY, 0, 200},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 220},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 100},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 240},
    {FREQ_SET, 1, 349},
    {DELAY, 0, 300},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    /* HOOK */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_HOOK_START},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 200},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 293},
    {DELAY, 0, 240},
    {FREQ_SET, 1, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 220},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 240},
    {FREQ_SET, 1, 349},
    {DELAY, 0, 300},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 200},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 293},
    {DELAY, 0, 240},
    {FREQ_SET, 1, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 220},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 240},
    {FREQ_SET, 1, 349},
    {DELAY, 0, 300},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    /* HOOKEND */
    {LOOP_POINT_SET, 0, LOOP_POINT_TYPE_HOOK_END},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 200},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 293},
    {DELAY, 0, 240},
    {FREQ_SET, 1, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 220},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 261},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 240},
    {FREQ_SET, 1, 349},
    {DELAY, 0, 300},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 180},
    {FREQ_SET, 1, 329},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 2, 329},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 2, 329},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 100},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 240},
    {FREQ_SET, 2, 293},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 349},
    {DELAY, 0, 120},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 200},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 120},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 200},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 2, 329},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 100},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 240},
    {FREQ_SET, 2, 293},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 349},
    {DELAY, 0, 120},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 220},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 160},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 4, 1108},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 240},
    {FREQ_SET, 2, 293},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 329},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 2, 329},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 100},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 240},
    {FREQ_SET, 2, 293},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 349},
    {DELAY, 0, 120},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 220},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 160},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 240},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 120},
    {FREQ_SET, 2, 329},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 80},
    {FREQ_SET, 2, 0},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 160},
    {FREQ_SET, 2, 329},
    {FREQ_SET, 3, 55},
    {DELAY, 0, 100},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 2, 329},
    {DELAY, 0, 240},
    {FREQ_SET, 2, 293},
    {DELAY, 0, 80},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 349},
    {DELAY, 0, 120},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 220},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 2, 0},
    {DELAY, 0, 100},
    {FREQ_SET, 3, 58},
    {DELAY, 0, 120},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 160},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 80},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 20},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {FREQ_SET, 3, 65},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 4, 1108},
    {DELAY, 0, 40},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 0},
    {DELAY, 0, 60},
    {FREQ_SET, 1, 220},
    {DELAY, 0, 60},
    {FREQ_SET, 3, 0},
    {DELAY, 0, 140},
    {FREQ_SET, 1, 0},
    {FREQ_SET, 4, 0},
    {DELAY, 0, 40},
    {FREQ_SET, 3, 73},
    {DELAY, 0, 200},
    {FREQ_SET, 3, 0},
};