#pragma once
#include <stdint.h>
#include <stddef.h>

typedef enum melody_item_type : uint8_t {
    FREQ_SET, // or 0 to turn off
    DUTY_SET,
    DELAY,
    LOOP_POINT_SET,
    SAMPLE_LOAD,
} melody_item_type_t;

enum loop_point_type : uint8_t {
    LOOP_POINT_TYPE_LOOP = 0,
    LOOP_POINT_TYPE_HOOK_START,
    LOOP_POINT_TYPE_HOOK_END
};

typedef struct __attribute__((packed)) melody_item {
    const melody_item_type_t command : 4;
    const uint8_t channel : 4;
    int argument;
} melody_item_t;

typedef enum mixing_mode {
    MIX_MODE_ADD,
    MIX_MODE_XOR,
} mixing_mode_t;

typedef struct rle_sample {
    const uint16_t sample_rate;
    const uint16_t root_frequency;
    const size_t length;
    const mixing_mode_t mode;
    /// @brief RLE data of the PWM audio sample. First byte is number of 1 bits, second byte is number of 0 bits that follow, and so forth.
    const uint8_t* rle_data;
} rle_sample_t;
