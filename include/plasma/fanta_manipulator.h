#pragma once
#include "sprite.h"
#include "font.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class FantaManipulator {
public:
    FantaManipulator(fanta_buffer_t, size_t, int width, SemaphoreHandle_t, bool* dirtyFlag);
    ~FantaManipulator();

    FantaManipulator* slice(int x, int width);

    void clear();
    void plot_pixel(int x, int y, bool state);
    void put_sprite(const sprite_t*, int x, int y);
    void put_glyph(const font_definition_t * font, const unsigned char glyph, int x, int y);
    void scroll(int dx, int dy);

    int get_width();
private:
    fanta_buffer_t buffer;
    size_t buffer_size;
    SemaphoreHandle_t buffer_semaphore;
    bool * dirty;
    int width;
};