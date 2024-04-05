#pragma once
#include <plasma/font.h>
#include <plasma/framebuffer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class Console {
public:
    Console(const font_definition_t*, PlasmaDisplayFramebuffer*);
    ~Console();

    void set_active(bool);
    void clear();
    void write(char);
    void print(const char *, ...);
    void set_cursor(bool);
    void set_font(const font_definition_t*);

    void task();

private:
    PlasmaDisplayFramebuffer * disp;
    const font_definition_t * font;
    QueueHandle_t hQueue;
    TaskHandle_t hTask;
    int cursor_x, cursor_y;
    volatile bool cursor_enable;
    volatile bool cursor_state;
    bool active;

    void cursor_newline();
};