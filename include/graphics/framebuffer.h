#pragma once
#include <graphics/display_driver.h>
#include "fanta_manipulator.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

/// @brief A framebuffer for driving the plasma display
class DisplayFramebuffer {
public:
    static const int width = 101;
    static const int height = 16;
    /// @brief Size of the backing buffer in bytes
    static const size_t PDFB_BUFFER_SIZE = width * (height / 8);

    DisplayFramebuffer(DisplayDriver * display);
    ~DisplayFramebuffer();

    /// @brief Clears the display
    void clear();

    /// @brief Halt execution of the current task until the next frame was fully output to the display controller
    void wait_next_frame();
    /// @brief Output the backing buffer to the display controller, if it was altered since the last transmission
    void write_all_if_needed();
    
#ifdef PDFB_PERF_LOGS
    /// @brief Get the average number of frames transmitted to the display controller per second
    unsigned int get_fps();
#endif

    /// @brief Manipulate the graphics in the backing buffer
    /// @return A pointer to the manipulator, owned by the caller
    FantaManipulator * manipulate();

private:
    uint8_t buffer[PDFB_BUFFER_SIZE];
    SemaphoreHandle_t buffer_semaphore;
    EventGroupHandle_t vsync_group;
    TaskHandle_t hTask;
    DisplayDriver * display;
    FantaManipulator * shared_manipulator;
    bool is_dirty;

    void setup_task();
    void write_all();
};