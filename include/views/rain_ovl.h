#pragma once
#include <plasma/fanta_manipulator.h>

typedef struct __attribute__((__packed__)) rain_particle {
    uint8_t x;
    uint8_t y;
    int8_t vx;
    uint8_t vy;
} rain_particle_t;

class RainOverlay {
public:
    RainOverlay(FantaManipulator*);
    void render();

    void set_windspeed(int8_t);
    void set_gravity(uint8_t);
    void set_intensity(uint8_t);
private:
    FantaManipulator * framebuffer;
    static const size_t PARTICLE_COUNT = 25;
    rain_particle_t particles[PARTICLE_COUNT];
    int8_t windspeed;
    uint8_t gravity;
    uint8_t intensity;
    uint8_t framecounter;
};