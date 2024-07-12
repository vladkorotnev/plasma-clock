#pragma once
#include <views/framework.h>
#include <sensor/sensor.h>

/// @brief A renderable that shows touch arrow hints
class TouchArrowOverlay: public Renderable {
public:
    bool top;
    bool bottom;
    bool left;
    bool right;

    TouchArrowOverlay(SensorPool* s) {
        active = false;
        top = false;
        bottom = false;
        left = false;
        right = false;
        sensors = s;
        phase = 0;
        framecount = 0;
        dir = true;
    }

    void render(FantaManipulator* fb) {
        static const uint8_t arrow_top_data[] = {
            0b0000000,
            0b0001000,
            0b0011100,
            0b0101010,
            0b0001000,
            0b0001000,
            0b0001000,
            0b0000000
        };
        static const uint8_t arrow_top_mask_data[] = {
            0b0001000,
            0b0011100,
            0b0111110,
            0b1111111,
            0b0111110,
            0b0011100,
            0b0011100,
            0b0011100
        };
        static const sprite_t arrow_top = { 
            .width = 7,
            .height = 8, 
            .data = arrow_top_data,
            .mask = arrow_top_mask_data
        };

        static const uint8_t arrow_bottom_data[] = {
            0b0000000,
            0b0001000,
            0b0001000,
            0b0001000,
            0b0101010,
            0b0011100,
            0b0001000,
            0b0000000
        };
        static const uint8_t arrow_bottom_mask_data[] = {
            0b0011100,
            0b0011100,
            0b0011100,
            0b0111110,
            0b1111111,
            0b0111110,
            0b0011100,
            0b0001000
        };
        static const sprite_t arrow_bottom = { 
            .width = 7, 
            .height = 8, 
            .data = arrow_bottom_data,
            .mask = arrow_bottom_mask_data
        };

        static const uint8_t arrow_right_data[] = {
            0b00000000,
            0b00001000,
            0b00000100,
            0b01111110,
            0b00000100,
            0b00001000,
            0b00000000,
        };
        static const uint8_t arrow_right_mask_data[] = {
            0b00001000,
            0b00011100,
            0b01111110,
            0b11111111,
            0b01111110,
            0b00011100,
            0b00001000,
        };
        static const sprite_t arrow_right = { 
            .width = 8,
            .height = 7, 
            .data = arrow_right_data,
            .mask = arrow_right_mask_data
        };

        static const uint8_t arrow_left_data[] = {
            0b00000000,
            0b00010000,
            0b00100000,
            0b01111110,
            0b00100000,
            0b00010000,
            0b00000000,
        };
        static const uint8_t arrow_left_mask_data[] = {
            0b00001000,
            0b00111000,
            0b01111110,
            0b11111111,
            0b01111110,
            0b00111000,
            0b00010000,
        };
        static const sprite_t arrow_left = { 
            .width = 8,
            .height = 7, 
            .data = arrow_left_data,
            .mask = arrow_left_mask_data
        };

        if(!active) return;

        if(top) {
            fb->put_sprite(&arrow_top, fb->get_width()/2 - arrow_top.width/2, phase - 1);
        }
        if(bottom) {
            fb->put_sprite(&arrow_bottom, fb->get_width()/2 - arrow_bottom.width/2, fb->get_height() - arrow_bottom.height - phase + 1);
        }
        if(left) {
            fb->put_sprite(&arrow_left, phase - 1, fb->get_height()/2 - arrow_left.height/2);
        }
        if(right) {
            fb->put_sprite(&arrow_right, fb->get_width() - arrow_right.width - phase + 1, fb->get_height()/2 - arrow_right.height/2);
        }

        framecount += 1;
        if(framecount == 6) {
            framecount = 0;
            if(dir) phase += 1;
            else phase -= 1;

            if(phase == 2) {
                dir = false;
                framecount = -12;
            }
            else if(phase == 0) {
                dir = true;
                framecount = -12;
            }
        }
    }

    void step() {
        sensor_info_t * sense = sensors->get_info(VIRTSENSOR_ID_HID_STARTLED);
        if(sense) {
            active = sense->last_result;
            if(!active) {
                phase = 0;
                framecount = -6;
                dir = true;
            }
        }
    }

private:
    bool active;
    uint8_t phase;
    int framecount;
    bool dir;
    SensorPool * sensors;
};