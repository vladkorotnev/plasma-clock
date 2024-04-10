#pragma once
#include <views/framework.h>

class ThunderOverlay: public Renderable {
public:
    ThunderOverlay(int width, int height);
    void set_active(bool);
    void render(FantaManipulator*);
    void step();

private:
    bool active;
    bool idle;
    int width;
    int height;
    int current_spawnpoint;
    int8_t current_path[16];
    int current_length;
    int max_delay_ms;
    int min_delay_ms;
    TickType_t last_bolted;
    uint32_t random_seed;
    int frame_counter;
};