#pragma once
#include <sound/beeper.h>

typedef struct melody_item {
    /// @brief Frequency of the note in Hz. 0 for a pause.
    int frequency;
    /// @brief Length of the note in ms.
    int length;
} melody_item_t;

typedef struct melody_sequence {
    const melody_item_t * array;
    size_t count;
} melody_sequence_t;

#define SEQUENCER_REPEAT_INDEFINITELY -1
#define SEQUENCER_NO_REPEAT 0

class BeepSequencer {
public:
    BeepSequencer(Beeper*);
    ~BeepSequencer();

    void play_sequence(const melody_sequence_t, beeper_channel_t, int repeat);
    void stop_sequence();
    /// @brief Block the current task until the playback has finished.
    void wait_end_play();

    void task();
private:
    Beeper * beeper;
    melody_sequence_t current_sequence;
    beeper_channel_t current_channel;
    size_t pointer;
    bool is_running;
    int repetitions;
    TaskHandle_t hTask;
    EventGroupHandle_t waitEndGroup;

    void start_task();
    void end_task();
};