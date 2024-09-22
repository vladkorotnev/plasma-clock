#pragma once
#include <graphics/fanta_manipulator.h>
#include <graphics/framebuffer.h>
#include <views/common/view.h>

typedef struct __attribute__((__packed__)) rain_particle {
    uint8_t x;
    uint8_t y;
    int8_t vx;
    uint8_t vy;
} rain_particle_t;

/// @brief A particle system view resembling rainfall or snowfall
class RainOverlay: public Renderable {
public:
    RainOverlay(int width = DisplayFramebuffer::width, int height = DisplayFramebuffer::height);
    void render(FantaManipulator*);
    void step();

    void set_windspeed(int8_t, bool variative);
    void set_gravity(uint8_t, bool variative);
    void set_intensity(uint8_t);
    void set_speed_divisor(uint8_t);
private:
    static const size_t PARTICLE_COUNT = 25;
    rain_particle_t particles[PARTICLE_COUNT];
    int8_t windspeed;
    uint8_t gravity;
    uint8_t intensity;
    uint8_t framecounter;
    uint8_t speed_divisor;
    int width;
    int height;
    bool windspeed_variative;
    bool gravity_variative;
};