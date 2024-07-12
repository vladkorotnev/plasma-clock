#include <views/overlays/thunder_ovl.h>

ThunderOverlay::ThunderOverlay(int w, int h) {
    width = w;
    height = std::min(h, 16);
    max_delay_ms = 7000;
    min_delay_ms = 2000;
    current_spawnpoint = 0;
    memset(current_path, 0, sizeof(current_path));
    current_length = 0;
    last_bolted = xTaskGetTickCount();
    active = false;
    idle = true;
    frame_counter = 0;
}

void ThunderOverlay::set_active(bool a) {
    active = a;
}

void ThunderOverlay::step() {
    if(!active) return;

    if(idle) {
        random_seed = esp_random();
        TickType_t now = xTaskGetTickCount();
        if(
            (now - last_bolted >= pdMS_TO_TICKS(max_delay_ms) 
            || (((random_seed & 0xFF00) >> 8) != 0 && ((random_seed & 0x00FF) % ((random_seed & 0xFF00) >> 8)) == 0))
            && now - last_bolted >= pdMS_TO_TICKS(min_delay_ms)
        ) {
            idle = false;
            current_spawnpoint = random_seed % (width + 1);
            memset(current_path, 0, sizeof(current_path));
            current_length = 1;
            random_seed = esp_random();
            frame_counter = 0;
        }
    } else if(current_length < height) {
        int8_t current_motion = 0;
        if(random_seed & 1) {
            current_motion = 1;
            random_seed >>= 1;
            if(random_seed & 1) {
                current_motion = -1;
            }
        }
        random_seed >>= 1;
        
        current_path[current_length] = current_motion;
        current_length++;
        if(current_motion != 0 && (random_seed & 1) == 0) {
            // amplify corners a bit
            current_path[current_length] = current_motion;
            current_length++;
        }
        frame_counter++;
    } else {
        frame_counter++;
        if(frame_counter >= height + 10) {
            idle = true;
            current_length = 0;
            last_bolted = xTaskGetTickCount();
        }
    }
}

void ThunderOverlay::render(FantaManipulator * fb) {
    if(!active || idle) return;

    int cur_x = current_spawnpoint;
    for(int cur_y = frame_counter % height; cur_y < current_length; cur_y++) {
        cur_x += current_path[cur_y];
        fb->plot_pixel(cur_x, cur_y, true);
    }

    if((random_seed > 0 && frame_counter % random_seed == 0) || (current_length >= height && frame_counter % 10 == 0)) {
        fb->invert();
    }
}