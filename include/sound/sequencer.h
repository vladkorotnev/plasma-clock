#pragma once
#include <sound/beeper.h>
#include <sound/generators.h>

typedef enum melody_item_type {
    FREQ_SET, // or 0 to turn off
    DELAY,
    LOOP_POINT_SET,
    MAX_INVALID
} melody_item_type_t;

typedef struct melody_item {
    const melody_item_type_t command : 4;
    const uint8_t channel : 4;
    const int argument1;
} melody_item_t;

typedef struct melody_sequence {
    const melody_item_t * array;
    size_t count;
} melody_sequence_t;

#define SEQUENCER_REPEAT_INDEFINITELY -1
#define SEQUENCER_NO_REPEAT 0

class NewSequencer: public WaveGenerator {
public:
    NewSequencer();
    ~NewSequencer();
    void play_sequence(const melody_sequence_t *, int repeat);
    void stop_sequence();
    void wait_end_play();
    size_t fill_buffer(void* buffer, size_t length) override;
    
private:
    static const int CHANNELS = 4;
    ToneGenerator * voices[CHANNELS] = { nullptr };
    const melody_sequence_t * sequence;
    size_t pointer;
    size_t loop_point;
    int repetitions;
    bool is_running;
    EventGroupHandle_t wait_end_group;
    int remaining_delay_samples;
    void process_steps_until_delay();
};
