#include <views/overlays/fireworks.h>

static char LOG_TAG[] = "FIWK";

FireworksOverlay::FireworksOverlay(Beeper * b, uint8_t w, uint8_t h): 
    width(w),
    height(h),
    active(false),
    particles_array(nullptr),
    sound(b)
{

}

void FireworksOverlay::set_active(bool a) {
    if(a && !active) { 
        make_memory_if_needed();
        last_launch = xTaskGetTickCount();
    }
    active = a;
}

void FireworksOverlay::make_memory_if_needed() {
    if(particles_array != nullptr) return;
    ESP_LOGI(LOG_TAG, "Making memory");
    particles_array = (FireworkParticle*) calloc(sizeof(FireworkParticle), MAX_PARTICLES);
    if(particles_array == nullptr) {
        ESP_LOGE(LOG_TAG, "Failed to allocate memory");
    }
}

void FireworksOverlay::cleanup() {
}

void FireworksOverlay::render(FantaManipulator * fb) {
    if(particles_array == nullptr) return;

    bool need_invert = false;

    for(int i = 0; i < MAX_PARTICLES; i++) {
        const FireworkParticle p = particles_array[i];
        switch(p.type) {
            case SHOOTING:
                fb->plot_pixel(p.x, p.y + 1, true);
                fb->plot_pixel(p.x, p.y, true);
                break;

            case EXPLODING:
                fb->plot_pixel(p.x, p.y, true);
                if(p.phase == 0) need_invert = true;
                break;

            case DOWNFALL:
                if(p.phase % 2 == 0) {
                    // downfall is slightly blinking
                    fb->plot_pixel(p.x, p.y, true);
                }
                break;

            default:
                break;
        }
    }

    if(need_invert) {
        // flash due to an explosion
        fb->invert();
    }
}

void FireworksOverlay::step() {
    if(particles_array == nullptr) return;
    bool has_active_particles = false;
    uint32_t rnd = esp_random();
    bool did_sound = false;

    for(int i = 0; i < MAX_PARTICLES; i++) {
        FireworkParticle * p = &particles_array[i];
        if(p->type != INACTIVE) {
            has_active_particles = true;

            p->x += p->vx;
            p->y += p->vy;
            p->phase += 1;

            switch(p->type) {
                case SHOOTING:
                    // shooting rocket, accelerates upwards every other frame, no horizontal motion
                    // explodes when reaching some height
                    if(p->phase % 2 == 0) {
                        p->vy = std::max(-3, p->vy - 1);
                    }
                    else if((((rnd & 0xF0F1) % 16) == 1 && p->y < height / 2) || p->y < height / 3) {
                        // EKUSUPUROOJONN!
                        unsigned debris_count = 16 + (rnd % 10);
                        unsigned debris_max = debris_count;
                        bool expl_phase = false;
                        uint32_t expl_seed = ((rnd & 0xFF00) >> 8) | (rnd & 0x00FF);
                        static const int8_t veltab_x[] = { 2, 1, 0, 1, 1, 2, 2 };
                        static const int8_t veltab_y[] = { 0, 1, 1, 0, 1, 1, 1};
                        p->type = INACTIVE; // will be reused in an instant
                        for(int j = 0; j < MAX_PARTICLES; j++) {
                            FireworkParticle * debris = &particles_array[j];
                            if(debris->type == INACTIVE) {
                                debris->x = p->x;
                                debris->y = p->y;
                                debris->vx = 0; debris->vy = 0;
                                int att = 10;
                                do {
                                    debris->vy = veltab_y[(debris_max - debris_count - (expl_phase ? 1 : 0)) % 7] - (expl_seed & 0x1);
                                    debris->vx = veltab_x[(debris_max - debris_count - (expl_phase ? 1 : 0)) % 7] - (((expl_seed >> 3) & 0xF) % 2);
                                    expl_seed = ((expl_seed & 0b111) << 29) | (expl_seed >> 3);
                                    att--;
                                } while(debris->vx == 0 && debris->vy == 0 && att > 0);
                                if(expl_phase) {
                                    debris->vx = -debris->vx;
                                    debris->vy = -debris->vy;
                                }
                                debris->phase = 0;
                                debris->type = EXPLODING;
                                debris_count--;

                                if(expl_phase) {
                                    expl_phase = false;
                                    expl_seed >>= 7;
                                    if(expl_seed == 0) {
                                        expl_seed = esp_random();
                                    }
                                } else {
                                    expl_phase = true;
                                }
                            }
                            if(debris_count == 0) break;
                        }
                    }
                    break;

                case EXPLODING:
                    if(p->phase == 1) {
                        if(sound != nullptr && !did_sound) {
                            sound->beep_blocking(CHANNEL_NOTICE, 100, 10);
                            did_sound = true;
                        }
                    } else if(p->phase >= 16 && p->phase % (((rnd >> 4) & 0xF) + 2) == 0) {
                        p->type = DOWNFALL;
                        p->phase = (rnd >> 8) & 0x3;
                        p->vy /= 2;
                    } else if(p->phase % 10 == 0 && p->vy < 2) {
                        p->vy +=  1;
                    }
                    break;

                case DOWNFALL:
                    p->vy = (p->phase % 2) + ((rnd >> 30) % 2);
                    if(p->y > height || (p->y < 0 && p->vy < 0) || p->y < -height/2) {
                        p->type = INACTIVE;
                    }
                    break;
            }
        } else {
            if(active) {
                TickType_t now = xTaskGetTickCount();
                if(
                    (
                        now - last_launch > max_delay || 
                        (((rnd & 0xF0F0) >> 4) % (intense ? 16 : rnd)) == 0
                    )
                    && now - last_launch > min_delay
                ) {
                    last_launch = now;
                    p->phase = 0;
                    p->type = SHOOTING;
                    p->y = height;
                    p->x = ((rnd & 0x3939) % (width - 4)) + 4;
                    p->vx = 0;
                    p->vy = -1;
                    rnd = esp_random();
                }
            }
        }
    }

    if(!active && !has_active_particles) {
        cleanup(); // release memory when not active and no remaining fireworks
    }
}