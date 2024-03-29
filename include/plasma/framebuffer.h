#pragma once
#include "iface.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
class PlasmaDisplayFramebuffer {
public:
    static const int width = 101;
    static const int height = 16;

    PlasmaDisplayFramebuffer(
        PlasmaDisplayIface * display
    );
    ~PlasmaDisplayFramebuffer();

    void clear();

    void wait_next_frame();
    void plot_pixel(int x, int y, bool state);

    inline void write_all_if_needed();

private:
    static const size_t PDFB_BUFFER_SIZE = width * (height / 8);
    uint8_t buffer[PDFB_BUFFER_SIZE];
    SemaphoreHandle_t buffer_semaphore;
    EventGroupHandle_t vsync_group;
    TaskHandle_t hTask;
    PlasmaDisplayIface * display;
    volatile bool is_dirty;

    void setup_task();
    void write_all();
};