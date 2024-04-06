#pragma once
#include <sound/beeper.h>

typedef struct melody_item {
    int frequency;
    int length;
} melody_item_t;

typedef struct melody_sequence {
    const melody_item_t * array;
    size_t count;
} melody_sequence_t;

class BeepSequencer {
public:
    BeepSequencer(Beeper*);
    ~BeepSequencer();

    void play_sequence(const melody_sequence_t, beeper_channel_t, int repeat);
    void stop_sequence();
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