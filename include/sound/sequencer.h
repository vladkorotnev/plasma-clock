#pragma once
#include <sound/beeper.h>
#include <sound/generators.h>
#include <sound/sequence.h>
#include <device_config.h>

typedef enum sequence_playback_flags : uint8_t {
    SEQUENCER_PLAY_NORMALLY = 0,
    SEQUENCER_REPEAT_INDEFINITELY = (1 << 0),
    SEQUENCER_PLAY_HOOK_ONLY = (1 << 1),
} sequence_playback_flags_t;

class NewSequencer: public WaveGenerator {
public:
    static const int TONE_CHANNELS = 4;
    static const int EXTRA_CHANNELS = 2;
    static const int CHANNELS = TONE_CHANNELS + EXTRA_CHANNELS;
    sequence_playback_flags_t flags;
    ToneGenerator * voices[CHANNELS] = { nullptr };
    NewSequencer();
    ~NewSequencer();
    void play_sequence(MelodySequence *, sequence_playback_flags_t flags = SEQUENCER_PLAY_NORMALLY, int repeat = 0);
    void stop_sequence();
    void wait_end_play();
    bool is_sequencing();
    size_t fill_buffer(void* buffer, size_t length) override;
#if HAS(SERIAL_MIDI)
    void midi_task();
#endif
    
private:
    MelodySequence * sequence;
    int num_rows;
    const melody_item_t * rows;
    size_t pointer;
    size_t loop_point;
    int repetitions;
    bool is_running;
    EventGroupHandle_t wait_end_group;
    int remaining_delay_samples;
    void process_steps_until_delay();
    bool process_step(const melody_item_t *);
    bool end_of_song();
    void find_hook();
#if HAS(SERIAL_MIDI)
    TaskHandle_t hMidiTask;
    int lastNote[CHANNELS] = { 0 };
#endif
};
