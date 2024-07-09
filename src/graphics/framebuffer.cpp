#include <graphics/framebuffer.h>
#include <string.h>
#include <Arduino.h>
#include <freertos/task.h>

static const int DESIRED_FPS = 65;

#define EVT_BIT_ENDED_DRAWING (1 << 0)

static char LOG_TAG[] = "PDFB";

#define LOCK_BUFFER_OR_DIE if(!xSemaphoreTake(buffer_semaphore, pdMS_TO_TICKS(16))) {ESP_LOGW(LOG_TAG, "Timeout while waiting on FB semaphore");return;}
#define UNLOCK_BUFFER xSemaphoreGive(buffer_semaphore)

DisplayFramebuffer::DisplayFramebuffer(DisplayDriver * disp) {
    display = disp;
    buffer_semaphore = xSemaphoreCreateBinary();
    vsync_group = xEventGroupCreate();
    xSemaphoreGive(buffer_semaphore);
    is_dirty = false;
    shared_manipulator = new FantaManipulator(buffer, PDFB_BUFFER_SIZE, width, height, buffer_semaphore, &is_dirty);
    setup_task();
    clear();
}

DisplayFramebuffer::~DisplayFramebuffer() {
    if(hTask != NULL) {
        ESP_LOGV(LOG_TAG, "Stopping task");
        vTaskDelete(hTask);
    }
}

extern "C" void FbTaskFunc( void * pvParameter );

#ifdef PDFB_PERF_LOGS
static TickType_t last_draw_at = 0;
static TickType_t avg_frametime = 0;
static uint16_t perf_counter = 0;

unsigned int DisplayFramebuffer::get_fps() {
    return 1000/pdTICKS_TO_MS(avg_frametime);
}
#endif

void FbTaskFunc( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "Running task");
    DisplayFramebuffer * fb = static_cast<DisplayFramebuffer*> ( pvParameter );

    while(1) {
        fb->write_all_if_needed();
#ifdef PDFB_PERF_LOGS
#ifndef PDFB_PERF_LOG_INTERVAL
#define PDFB_PERF_LOG_INTERVAL 600
#endif
        perf_counter++;
        if(perf_counter >= PDFB_PERF_LOG_INTERVAL) {
            perf_counter = 0;
            ESP_LOGV(LOG_TAG, "Avg frametime=%lu, FPS=%lu", pdTICKS_TO_MS(avg_frametime), 1000/pdTICKS_TO_MS(avg_frametime));
        }

        TickType_t now = xTaskGetTickCount();
        avg_frametime += (now - last_draw_at);
        avg_frametime /= 2;
        last_draw_at = now;
#endif

        vTaskDelay(pdMS_TO_TICKS(1000 / DESIRED_FPS));
    }
}

void DisplayFramebuffer::setup_task() {
    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        FbTaskFunc,
        "FBuf",
        4096,
        this,
        configMAX_PRIORITIES - 2,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

void DisplayFramebuffer::wait_next_frame() {
    xEventGroupClearBits(vsync_group, EVT_BIT_ENDED_DRAWING);
    xEventGroupWaitBits(vsync_group, EVT_BIT_ENDED_DRAWING, false, true, portMAX_DELAY);
}

void DisplayFramebuffer::clear() {
    shared_manipulator->clear();
}

void DisplayFramebuffer::write_all() {
    LOCK_BUFFER_OR_DIE;
    display->write_fanta(buffer, PDFB_BUFFER_SIZE);
    is_dirty = false;
    UNLOCK_BUFFER;
    xEventGroupSetBits(vsync_group, EVT_BIT_ENDED_DRAWING);
}

void DisplayFramebuffer::write_all_if_needed() {
    if(is_dirty) write_all();
    else xEventGroupSetBits(vsync_group, EVT_BIT_ENDED_DRAWING);
}

FantaManipulator* DisplayFramebuffer::manipulate() {
    return shared_manipulator;
}
