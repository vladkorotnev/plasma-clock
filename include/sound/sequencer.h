#pragma once
#include <sound/beeper.h>
#include <sound/generators.h>
#include <device_config.h>

extern const rle_sample_t kick_sample;

typedef enum melody_item_type {
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

typedef struct melody_item {
    const melody_item_type_t command : 4;
    const uint8_t channel : 4;
    int argument;
} melody_item_t;

typedef struct melody_sequence {
    const melody_item_t * array;
    size_t count;
} melody_sequence_t;

typedef enum sequence_playback_flags {
    SEQUENCER_PLAY_NORMALLY = 0,
    SEQUENCER_REPEAT_INDEFINITELY = (1 << 0),
    SEQUENCER_PLAY_HOOK_ONLY = (1 << 1),
    
} sequence_playback_flags_t;

class NewSequencer: public WaveGenerator {
public:
    static const int TONE_CHANNELS = 4;
    static const int EXTRA_CHANNELS = 2;
    static const int CHANNELS = TONE_CHANNELS + EXTRA_CHANNELS;
    ToneGenerator * voices[CHANNELS] = { nullptr };
    NewSequencer();
    ~NewSequencer();
    void play_sequence(const melody_sequence_t *, sequence_playback_flags_t flags = SEQUENCER_PLAY_NORMALLY, int repeat = 0);
    void stop_sequence();
    void wait_end_play();
    bool is_sequencing();
    size_t fill_buffer(void* buffer, size_t length) override;
#if HAS(SERIAL_MIDI)
    void midi_task();
#endif
    
private:
    const melody_sequence_t * sequence;
    size_t pointer;
    size_t loop_point;
    int repetitions;
    bool is_running;
    sequence_playback_flags_t flags;
    EventGroupHandle_t wait_end_group;
    int remaining_delay_samples;
    void process_steps_until_delay();
    bool process_step(const melody_item_t *);
    bool end_of_song();
    void find_hook();
#if HAS(SERIAL_MIDI)
    TaskHandle_t hMidiTask;
#endif
};
