#pragma once
#include "iface.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class PlasmaDisplayFramebuffer {
public:
    PlasmaDisplayFramebuffer(
        PlasmaDisplayIface * display
    );

    void clear();

    void plot_pixel(int x, int y, bool state);

    inline void write_all_if_needed();

private:
    static const size_t PDFB_BUFFER_SIZE = 202;
    uint8_t buffer[PDFB_BUFFER_SIZE];
    SemaphoreHandle_t buffer_semaphore;
    TaskHandle_t hTask;
    PlasmaDisplayIface * display;
    volatile bool is_dirty;

    void setup_task();
    void write_all();
};