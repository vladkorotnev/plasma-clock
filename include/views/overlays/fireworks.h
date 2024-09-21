#pragma once
#include <views/framework.h>

class FireworksOverlay: public Renderable {
public:
    FireworksOverlay(Beeper *, uint8_t width = DisplayFramebuffer::width, uint8_t height = DisplayFramebuffer::height);
    TickType_t max_delay = pdMS_TO_TICKS(2000);
    TickType_t min_delay = pdMS_TO_TICKS(33);
    bool intense = false;

    void set_active(bool active);

    void render(FantaManipulator *) override;
    void step() override;
    void cleanup() override;

private:
    static const unsigned MAX_PARTICLES = 64;

    enum ParticleType {
        INACTIVE = 0,
        SHOOTING,
        EXPLODING,
        DOWNFALL
    };

    struct FireworkParticle {
        ParticleType type : 2;
        uint8_t phase : 6;
        int8_t y;
        int8_t x;
        int8_t vy;
        int8_t vx;
    };

    uint8_t width;
    uint8_t height;
    bool active;
    FireworkParticle * particles_array;
    Beeper * sound;
    TickType_t last_launch;

    void make_memory_if_needed();
};