#pragma once
#include <graphics/font.h>
#include <graphics/framebuffer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/// @brief A simple low level console that does not utilize the Renderable tooling
class Console {
public:
    Console(const font_definition_t*, DisplayFramebuffer*);
    ~Console();

    void set_active(bool);
    void clear();
    void write(char);
    void print(const char *, ...);
    void set_cursor(bool);
    void set_font(const font_definition_t*);

    void flush();

    void task();

private:
    DisplayFramebuffer * disp;
    const font_definition_t * font;
    QueueHandle_t hQueue;
    TaskHandle_t hTask;
    int cursor_x, cursor_y;
    volatile bool cursor_enable;
    volatile bool cursor_state;
    bool active;

    void cursor_newline(FantaManipulator*);
};