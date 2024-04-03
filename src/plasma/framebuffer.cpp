#include <plasma/framebuffer.h>
#include <string.h>
#include <Arduino.h>

static const int DESIRED_FPS = 63;

#define EVT_BIT_ENDED_DRAWING (1 << 0)

static char LOG_TAG[] = "PDFB";

#define LOCK_BUFFER_OR_DIE if(!xSemaphoreTake(buffer_semaphore, pdMS_TO_TICKS(16))) {ESP_LOGW(LOG_TAG, "Timeout while waiting on FB semaphore");return;}
#define UNLOCK_BUFFER xSemaphoreGive(buffer_semaphore)

PlasmaDisplayFramebuffer::PlasmaDisplayFramebuffer(PlasmaDisplayIface * disp) {
    display = disp;
    buffer_semaphore = xSemaphoreCreateBinary();
    vsync_group = xEventGroupCreate();
    xSemaphoreGive(buffer_semaphore);
    setup_task();
    clear();
}

PlasmaDisplayFramebuffer::~PlasmaDisplayFramebuffer() {
    if(hTask != NULL) {
        ESP_LOGV(LOG_TAG, "Stopping task");
        vTaskDelete(hTask);
    }
}

extern "C" void FbTaskFunc( void * pvParameter );

void FbTaskFunc( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "Running task");
    PlasmaDisplayFramebuffer * fb = static_cast<PlasmaDisplayFramebuffer*> ( pvParameter );
#ifdef PDFB_PERF_LOGS
    static TickType_t last_draw_at = 0;
    static TickType_t avg_frametime = 0;
    static uint16_t perf_counter = 0;
#endif

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

void PlasmaDisplayFramebuffer::setup_task() {
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

void PlasmaDisplayFramebuffer::wait_next_frame() {
    xEventGroupWaitBits(vsync_group, EVT_BIT_ENDED_DRAWING, false, true, portMAX_DELAY);
}

void PlasmaDisplayFramebuffer::clear() {
    LOCK_BUFFER_OR_DIE;

    memset(buffer, 0x0, PDFB_BUFFER_SIZE);
    is_dirty = true;

    UNLOCK_BUFFER;
}

void PlasmaDisplayFramebuffer::write_all() {
    LOCK_BUFFER_OR_DIE;

    xEventGroupClearBits(vsync_group, EVT_BIT_ENDED_DRAWING);
    for(int i = 0; i < PDFB_BUFFER_SIZE; i++) {
        display->write_stride(buffer[i]);
    }
    is_dirty = false;
    xEventGroupSetBits(vsync_group, EVT_BIT_ENDED_DRAWING);

    UNLOCK_BUFFER;
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

    LOCK_BUFFER_OR_DIE;

    if(state) {
        buffer[target_idx] |= target_bitmask;
    } else { 
        buffer[target_idx] &= ~target_bitmask;
    }

    is_dirty = true;

    UNLOCK_BUFFER;
}

void PlasmaDisplayFramebuffer::put_sprite(const sprite_t * sprite, int x, int y) {    
    LOCK_BUFFER_OR_DIE;

    uint8_t * fanta = sprite_to_fanta(sprite);

    uint16_t fanta_column_mask = 0x0;
    for(int i = y; i < y + sprite->height; i++) {
        if(i < 0) continue;
        if(i > 16) break;
        fanta_column_mask |= (1 << i);
    }

    fanta_offset_y(fanta, y, sprite->width);

    size_t target_idx = x * 2;
    uint8_t * mask_data = (uint8_t*) &fanta_column_mask;

    for(int i = 0; i < sprite->width*2; i++) {
        if(target_idx + i > PDFB_BUFFER_SIZE - 1) break;
        uint8_t current_mask = mask_data[i % 2];

        buffer[target_idx + i] = fanta[i] | (buffer[target_idx + i] & ~current_mask);
    }

    free(fanta);

    is_dirty = true;

    UNLOCK_BUFFER;
}

void PlasmaDisplayFramebuffer::put_glyph(const font_definition_t * font, const unsigned char glyph, int x, int y) {
    sprite_t char_sprite = sprite_from_glyph(font, glyph);
    put_sprite(&char_sprite, x, y);
}

void PlasmaDisplayFramebuffer::scroll(int dx, int dy) {
    LOCK_BUFFER_OR_DIE;

    if(dy != 0) {
        fanta_offset_y(buffer, dy, width);
    }

    if(dx > 0) {
        size_t dst_index = dx * 2;
        memcpy(&buffer[dst_index], buffer, PDFB_BUFFER_SIZE - dst_index);
        memset(buffer, 0, dst_index);
    } else if (dx < 0) {
        size_t src_index = abs(dx) * 2;
        memcpy(buffer, &buffer[src_index], PDFB_BUFFER_SIZE - src_index);
        memset(&buffer[src_index], 0, PDFB_BUFFER_SIZE - src_index);
    }

    UNLOCK_BUFFER;
}