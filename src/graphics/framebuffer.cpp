#include <graphics/framebuffer.h>
#include <string.h>
#include <Arduino.h>
#include <freertos/task.h>
#include <os_config.h>

#ifndef HWCONF_DESIRED_FPS
#define HWCONF_DESIRED_FPS 60
#endif

#ifndef PDFB_PERF_LOG_INTERVAL
#define PDFB_PERF_LOG_INTERVAL 600
#endif

static const int DESIRED_FPS = HWCONF_DESIRED_FPS;

#define EVT_BIT_ENDED_DRAWING (1 << 0)

static char LOG_TAG[] = "PDFB";

#define LOCK_BUFFER_OR_DIE if(!xSemaphoreTake(buffer_semaphore, portMAX_DELAY)) {ESP_LOGW(LOG_TAG, "Timeout while waiting on FB semaphore");return;}
#define UNLOCK_BUFFER xSemaphoreGive(buffer_semaphore)

DisplayFramebuffer::DisplayFramebuffer(DisplayDriver * disp) {
    display = disp;
    buffer_semaphore = xSemaphoreCreateBinary();
    vsync_group = xEventGroupCreate();
    xSemaphoreGive(buffer_semaphore);
    is_dirty = false;
    shared_manipulator = new FantaManipulator(buffer, BUFFER_SIZE, width, height, buffer_semaphore, &is_dirty);
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

static TickType_t last_draw_at = 0;
static TickType_t avg_frametime = 0;
static TickType_t max_frametime = 0;
static uint16_t perf_counter = 0;

unsigned int DisplayFramebuffer::get_fps() {
    return 1000/pdTICKS_TO_MS(avg_frametime);
}

void FbTaskFunc( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "Running task");
    DisplayFramebuffer * fb = static_cast<DisplayFramebuffer*> ( pvParameter );
    TickType_t now = xTaskGetTickCount();
    TickType_t interval = pdMS_TO_TICKS(1000 / DESIRED_FPS);
    while(1) {
        fb->write_all_if_needed();

        perf_counter++;
        if(perf_counter >= PDFB_PERF_LOG_INTERVAL) {
            perf_counter = 0;
            ESP_LOGI(LOG_TAG, "Avg frametime=%lu, FPS=%lu; MAX frametime=%lu, FPS=%lu", pdTICKS_TO_MS(avg_frametime), 1000/pdTICKS_TO_MS(avg_frametime), pdTICKS_TO_MS(max_frametime), 1000/pdTICKS_TO_MS(max_frametime));
            max_frametime = 0;
        }

        TickType_t frametime = now - last_draw_at;
        if(frametime > max_frametime) max_frametime = frametime;
        avg_frametime += frametime;
        avg_frametime /= 2;
        last_draw_at = now;

        xTaskDelayUntil(&now, interval); //<- also updates `now`
    }
}

void DisplayFramebuffer::setup_task() {
    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        FbTaskFunc,
        "FBuf",
        4096,
        this,
        pisosTASK_PRIORITY_FRAMEBUFFER,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

void DisplayFramebuffer::wait_next_frame() {
    xEventGroupClearBits(vsync_group, EVT_BIT_ENDED_DRAWING);
    xEventGroupWaitBits(vsync_group, EVT_BIT_ENDED_DRAWING, false, true, pdMS_TO_TICKS(1000));
}

void DisplayFramebuffer::clear() {
    shared_manipulator->clear();
}

void DisplayFramebuffer::write_all() {
    LOCK_BUFFER_OR_DIE;
    display->write_fanta(buffer, BUFFER_SIZE);
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
