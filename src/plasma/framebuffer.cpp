#include <plasma/framebuffer.h>
#include <string.h>
#include <Arduino.h>

static const int DESIRED_FPS = 60;
static char LOG_TAG[] = "PDFB";

PlasmaDisplayFramebuffer::PlasmaDisplayFramebuffer(PlasmaDisplayIface * disp) {
    display = disp;
    buffer_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(buffer_semaphore);
    setup_task();
    clear();
}

extern "C" void TaskFunction( void * pvParameter );

void TaskFunction( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "Running task");
    PlasmaDisplayFramebuffer * fb = static_cast<PlasmaDisplayFramebuffer*> ( pvParameter );
    while(1) {
        fb->write_all_if_needed();
        vTaskDelay(pdMS_TO_TICKS(1000 / DESIRED_FPS));
    }
}

void PlasmaDisplayFramebuffer::setup_task() {
    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        TaskFunction,
        "FBuf",
        4096,
        this,
        configMAX_PRIORITIES - 2,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

void PlasmaDisplayFramebuffer::clear() {
    if(!xSemaphoreTake(buffer_semaphore, pdMS_TO_TICKS(16))) {
        ESP_LOGW(LOG_TAG, "Timeout while waiting on FB semaphore");
        return;
    }

    memset(buffer, 0x0, PDFB_BUFFER_SIZE);
    is_dirty = true;

    xSemaphoreGive(buffer_semaphore);
}

void PlasmaDisplayFramebuffer::write_all() {
    if(buffer_semaphore == NULL) {
        ESP_LOGE(LOG_TAG, "Semaphore is null"); return;
    }

    if(!xSemaphoreTake(buffer_semaphore, pdMS_TO_TICKS(16))) {
        ESP_LOGW(LOG_TAG, "Timeout while waiting on FB semaphore");
        return;
    }

    for(int i = 0; i < PDFB_BUFFER_SIZE; i++) {
        display->write_stride(buffer[i]);
    }
    is_dirty = false;

    xSemaphoreGive(buffer_semaphore);
}

inline void PlasmaDisplayFramebuffer::write_all_if_needed() {
    if(!is_dirty) return;
    write_all();
}

void PlasmaDisplayFramebuffer::plot_pixel(int x, int y, bool state) {
    if(x >= PDFB_BUFFER_SIZE / 2 || y >= 16) {
        ESP_LOGE(LOG_TAG, "Position (X=%i, Y=%i) is out of bounds of the screen", x, y);
        return;
    }

    size_t target_idx = x * 2;
    if(y >= 8) {
        target_idx += 1;
        y -= 8;
    }
    uint8_t target_bitmask = 1 << y;

    if(!xSemaphoreTake(buffer_semaphore, pdMS_TO_TICKS(16))) {
        ESP_LOGW(LOG_TAG, "Timeout while waiting on FB semaphore");
        return;
    }

    if(state) {
        buffer[target_idx] |= target_bitmask;
    } else { 
        buffer[target_idx] &= ~target_bitmask;
    }

    is_dirty = true;
    xSemaphoreGive(buffer_semaphore);
}