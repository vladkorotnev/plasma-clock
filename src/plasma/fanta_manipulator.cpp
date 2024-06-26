#include <plasma/fanta_manipulator.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <Arduino.h>

static char LOG_TAG[] = "FMAN";

#define LOCK_BUFFER_OR_DIE if(!xSemaphoreTake(buffer_semaphore, pdMS_TO_TICKS(16))) {ESP_LOGW(LOG_TAG, "Timeout while waiting on FB semaphore");return;}
#define UNLOCK_BUFFER xSemaphoreGive(buffer_semaphore)

FantaManipulator::FantaManipulator(fanta_buffer_t buf, size_t size, int w, int h, SemaphoreHandle_t sema, bool* df) {
    buffer = buf;
    buffer_size = size;
    buffer_semaphore = sema;
    width = w;
    height = h;
    dirty = df;
}

FantaManipulator::~FantaManipulator() {
}

bool FantaManipulator::lock(TickType_t timeout) {
    return xSemaphoreTake(buffer_semaphore, timeout);
}

void FantaManipulator::unlock() {
    xSemaphoreGive(buffer_semaphore);
}

FantaManipulator* FantaManipulator::slice(int x, int w) {
    if(x > buffer_size/2) {
        ESP_LOGE(LOG_TAG, "Position (X=%i) is out of bounds of the screen", x);
        return NULL;
    }

    if(w > width - x) {
        w = width - x;
    }

    return new FantaManipulator(&buffer[x * 2], w * 2, w, height, buffer_semaphore, dirty);
} 

void FantaManipulator::clear() {
    memset(buffer, 0x0, buffer_size);
    *dirty = true;
}

void FantaManipulator::plot_pixel(int x, int y, bool state) {
    if(x < 0 || y < 0 || x > buffer_size/2 || y > 16) {
        return;
    }

    size_t target_idx = x * 2;
    if(y >= 8) {
        target_idx += 1;
        y -= 8;
    }
    uint8_t target_bitmask = 1 << y;

    if(state) {
        buffer[target_idx] |= target_bitmask;
    } else { 
        buffer[target_idx] &= ~target_bitmask;
    }

    *dirty = true;
}

void FantaManipulator::put_fanta(fanta_buffer_t fanta, int x, int y, int w, int h) {
    if(w <= 0 || h <= 0) return;
    
    uint16_t fanta_column_mask = 0x0;
    for(int i = y; i < y + h; i++) {
        if(i < 0) continue;
        if(i > 16) break;
        fanta_column_mask |= (1 << i);
    }

    fanta_offset_y(fanta, y, w);

    uint8_t * mask_data = (uint8_t*) &fanta_column_mask;

    size_t target_idx = x * 2;
    for(int i = 0; i < w*2; i++) {
        if(target_idx + i > buffer_size - 1) continue;
        if(target_idx + i < 0) continue;
        uint8_t current_mask = mask_data[i % 2];

        buffer[target_idx + i] = ((uint8_t) fanta[i]) | (buffer[target_idx + i] & ~current_mask);
    }

    *dirty = true;
}

void FantaManipulator::put_sprite(const sprite_t * sprite, int x, int y) {
    fanta_buffer_t fanta = sprite_to_fanta(sprite);
    put_fanta(fanta, x, y, sprite->width, sprite->height);
    free(fanta);
}

void FantaManipulator::put_glyph(const font_definition_t * font, const unsigned char glyph, int x, int y) {
    sprite_t char_sprite = sprite_from_glyph(font, glyph);
    put_sprite(&char_sprite, x, y);
}

void FantaManipulator::put_string(const font_definition_t * font, const char * string, int x, int y) {
    size_t i = 0;
    int cur_x = x;
    while(char ch = string[i]) {
        put_glyph(font, ch, cur_x, y);
        cur_x += font->width;
        i++;
    }
}

void FantaManipulator::scroll(int dx, int dy) {
    if(dy != 0) {
        fanta_offset_y(buffer, dy, width);
    }

    if(dx > 0) {
        size_t dst_index = dx * 2;
        memcpy(&buffer[dst_index], buffer, buffer_size - dst_index);
        memset(buffer, 0, dst_index);
    } else if (dx < 0) {
        size_t src_index = abs(dx) * 2;
        memcpy(buffer, &buffer[src_index], buffer_size - src_index);
        memset(&buffer[src_index], 0, buffer_size - src_index);
    }

    *dirty = true;
}

int FantaManipulator::get_width() {
    return width;
}

int FantaManipulator::get_height() {
    return height;
}

const fanta_buffer_t FantaManipulator::get_fanta() {
    return buffer;
}

void FantaManipulator::invert() {
    uint16_t* buffer_fast = (uint16_t*)buffer; // can't use uint32 since there is only 202 bytes in the whole screen, not divisible by 4
    size_t buffer_fast_size = buffer_size / (sizeof(uint16_t) / sizeof(buffer[0]));

    for(int i = 0; i < buffer_fast_size; i++) {
        buffer_fast[i] = ~buffer_fast[i];
    }
}