#pragma once
#include "sprite.h"
#include "font.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Why Fanta?
// Well if a normally stored (LTR in a byte, top-to-bottom within an array) graphical thingamajiggalo is called a "Sprite",
// then why can't a weirdly oriented one prepared to be drawn on an orange display be called a "Fanta"?

class FantaManipulator {
public:
    FantaManipulator(fanta_buffer_t, size_t, int width, int height, SemaphoreHandle_t, bool* dirtyFlag);
    ~FantaManipulator();

    FantaManipulator* slice(int x, int width);

    bool lock(TickType_t maxDelay = portMAX_DELAY);
    void unlock();

    void clear();
    void plot_pixel(int x, int y, bool state);
    void put_fanta(const fanta_buffer_t, int x, int y, int w, int h);
    void put_sprite(const sprite_t*, int x, int y);
    void put_glyph(const font_definition_t * font, const unsigned char glyph, int x, int y);
    void put_string(const font_definition_t *, const char *, int x, int y);
    void scroll(int dx, int dy);
    void invert();

    int get_width();
    int get_height();
    const fanta_buffer_t get_fanta();
private:
    fanta_buffer_t buffer;
    size_t buffer_size;
    SemaphoreHandle_t buffer_semaphore;
    bool * dirty;
    int width;
    int height;
};