#include <views/rain_ovl.h>
#include <esp_random.h>

#define PARTICLE_INACTIVE 0xFF

RainOverlay::RainOverlay(FantaManipulator* f) {
    framebuffer = f;
    windspeed = 0;
    gravity = 1;
    intensity = 10;
    speed_divisor = 1;
    memset(particles, PARTICLE_INACTIVE, PARTICLE_COUNT*sizeof(rain_particle_t));
}

void RainOverlay::set_gravity(uint8_t g) {
    gravity = g;
}

void RainOverlay::set_intensity(uint8_t i) {
    intensity = i;
    if(intensity > PARTICLE_COUNT) intensity = PARTICLE_COUNT;
    else for(int i = intensity; i < PARTICLE_COUNT; i++) particles[i].x = PARTICLE_INACTIVE;
}

void RainOverlay::set_windspeed(int8_t w) {
    windspeed = w;
}

void RainOverlay::set_speed_divisor(uint8_t d) {
    speed_divisor = d;
}

void RainOverlay::step() {
    framecounter++;
    if(framecounter % speed_divisor == 0) {
        for(int i = 0; i < intensity; i++) {
        rain_particle_t * p = &particles[i];
            if(p->x == PARTICLE_INACTIVE) {
                // (Re-)spawn a droplet on the top, but not right away
                uint32_t rnd = esp_random();
                if(rnd % 10 > 5) {
                    rnd >>= 4;
                    p->y = 0;
                    p->x = rnd % (framebuffer->get_width() + 1);
                    rnd >>= 4;
                    int8_t ws_offset = windspeed > 0 ? ((int8_t)rnd) % 2 : 0;
                    p->vx = windspeed + ws_offset;
                    p->vy = gravity;
                }
            } else if (p->y > framebuffer->get_height() + 2) {
                p->x = PARTICLE_INACTIVE;
            } else {
                // Advance the droplet            
                p->y += p->vy;
                p->x += p->vx;
                uint32_t rnd;
                if(p->y % 3 == 0) {
                    rnd = esp_random();
                    p->vy += ((int8_t) rnd) % 2;
                    p->vy += gravity;
                    rnd >>= 4;
                }
                if(p->x % 6 == 0 && windspeed > 0) {
                    p->vx += ((int8_t) rnd) % p->vx;
                }
            }
        }
    }
}

void RainOverlay::render() {
    for(int i = 0; i < intensity; i++) {
        rain_particle_t * p = &particles[i];

        framebuffer->plot_pixel(p->x, p->y, true);
        framebuffer->plot_pixel(p->x - (p->vx > 0 ? 1 :  p->vx < 0 ? -1 : 0), p->y - 1, true);
    }
}