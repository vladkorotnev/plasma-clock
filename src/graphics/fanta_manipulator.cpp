#include <graphics/fanta_manipulator.h>
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
    if(buffer_semaphore == NULL) return true;
    return xSemaphoreTake(buffer_semaphore, timeout);
}

void FantaManipulator::unlock() {
    if(buffer_semaphore == NULL) return;
    xSemaphoreGive(buffer_semaphore);
}

FantaManipulator* FantaManipulator::slice(int x, int w) {
    if(x > buffer_size/2) {
        ESP_LOGE(LOG_TAG, "Position (X=%i) is out of bounds of the screen", x);
        return nullptr;
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
    if(x < 0 || y < 0 || x >= width || y >= height) {
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

// NB: This still relies too much on the fact our displays are 16 bits tall. Someday when porting to another display this will bite us in the ass.
void FantaManipulator::put_fanta(const fanta_buffer_t fanta, int x, int y, int w, int h, fanta_buffer_t mask, bool invert) {
    if(w <= 0 || h <= 0) return;
    
    uint16_t fanta_column_mask = 0x0;
    for(int i = std::max(y, 0); i < std::min(y + h, 16); i++) {
        fanta_column_mask |= (1 << i);
    }

    uint8_t * mask_data = (uint8_t*) &fanta_column_mask;

    int target_idx = x * 2;
    for(int i = std::max(0, -target_idx); i < w*2; i++) {
        if(target_idx + i > buffer_size - 1) break;
        uint8_t current_halfcolumn_mask = mask_data[i % 2];
        if(mask) {
            uint16_t buf_mask_column = ((uint16_t*) mask)[i / 2];
            if(y > 0) buf_mask_column <<= y;
            else if(y < 0) buf_mask_column >>= abs(y);

            current_halfcolumn_mask &= ((uint8_t*)&buf_mask_column)[i % 2];
        }

        uint16_t buf_column = ((uint16_t*)fanta)[i / 2];
        if(y > 0) buf_column <<= y;
        else if(y < 0) buf_column >>= abs(y);
        uint8_t buf_halfcolumn = ((uint8_t*)&buf_column)[i % 2];
        buffer[target_idx + i] = ((uint8_t) (invert ? ~buf_halfcolumn : buf_halfcolumn) & current_halfcolumn_mask) | (buffer[target_idx + i] & ~current_halfcolumn_mask);
    }

    *dirty = true;
}

void FantaManipulator::put_horizontal_data(const uint8_t* data, int x, int y, int w, int h, const uint8_t* mask, bool invert) {
    if(w <= 0 || h <= 0) return;
    
    uint16_t fanta_column_mask = 0x0;
    for(int i = std::max(y, 0); i < std::min(y + h, 16); i++) {
        fanta_column_mask |= (1 << i);
    }

    uint8_t * mask_data = (uint8_t*) &fanta_column_mask;

    int target_idx = x * 2;
    for(int i = std::max(0, -target_idx); i < w*2; i++) {
        if(target_idx + i > buffer_size - 1) break;
        int i_spr = i / 2;
        uint8_t current_halfcolumn_mask = mask_data[i % 2];

        if(mask) {
            uint16_t buf_mask_column = 0;

            for(int j = std::min(h, 16) - 1; j >= 0; j--) {
                buf_mask_column <<= 1;
                uint8_t cur_byte = mask[j*std::max((w/8), 1) + i_spr / 8];
                if(w < 8) cur_byte <<= (8 - w);
                buf_mask_column |= (cur_byte >> (7 - (i_spr % 8))) & 0x1;
            }

            if(y > 0) buf_mask_column <<= y;
            else if(y < 0) buf_mask_column >>= abs(y);

            current_halfcolumn_mask &= ((uint8_t*)&buf_mask_column)[i % 2];
        }

        uint16_t buf_column = 0;
        for(int j = std::min(h, 16) - 1; j >= 0; j--) {
            buf_column <<= 1;
            uint8_t cur_byte = data[j*std::max((w/8), 1) + i_spr / 8];
            if(w < 8) cur_byte <<= (8 - w);
            buf_column |= (cur_byte >> (7 - (i_spr % 8))) & 0x1;
        }
        if(y > 0) buf_column <<= y;
        else if(y < 0) buf_column >>= abs(y);
        uint8_t buf_halfcolumn = ((uint8_t*)&buf_column)[i % 2];
        buffer[target_idx + i] = ((uint8_t) (invert ? ~buf_halfcolumn : buf_halfcolumn) & current_halfcolumn_mask) | (buffer[target_idx + i] & ~current_halfcolumn_mask);
    }

    *dirty = true;
}

void FantaManipulator::put_sprite(const sprite_t * sprite, int x, int y, bool invert) {
    switch(sprite->format) {
        case SPRFMT_NATIVE:
            put_fanta((const fanta_buffer_t) sprite->data, x, y, sprite->width, sprite->height, (const fanta_buffer_t) sprite->mask, invert);
            break;

        case SPRFMT_HORIZONTAL:
            put_horizontal_data(sprite->data, x, y, sprite->width, sprite->height, sprite->mask, invert);
            break;

        default:
            ESP_LOGE(LOG_TAG, "Unsupported sprfmt=%i !!", sprite->format);
            break;
    }
}

void FantaManipulator::put_glyph(const font_definition_t * font, const unsigned char glyph, int x, int y, text_attributes_t style) {
    if(x + font->width + ((style & TEXT_OUTLINED) ? 1 : 0) < 0 || x - ((style & TEXT_OUTLINED) ? 1 : 0) > get_width()) return;
    if(y + font->height + ((style & TEXT_OUTLINED) ? 1 : 0) < 0 || y - ((style & TEXT_OUTLINED) ? 1 : 0) > get_height()) return;

    bool need_masked_char = ((style & TEXT_NO_BACKGROUND != 0) || (style & TEXT_OUTLINED != 0));
    bool invert = (style & TEXT_INVERTED) != 0;

    static sprite_t char_sprite = { 0 };
    sprite_from_glyph(font, glyph, need_masked_char, &char_sprite);
    if(need_masked_char && (style & TEXT_NO_BACKGROUND) == 0) {
        // If the char is masked but background was requested, prepare the background on our own
        rect(x, y, x + font->width, y + font->height, true, invert);
    }

    if((style & TEXT_OUTLINED) != 0) {
        // Simplest way of drawing an outline is just 9 copies of the character of the opposite color
        // This is laggy if there is a lot of text going on, TODO optimize? Or better yet stop calling this directly and make a proper Label control
        for(int xoffs = -1; xoffs <= 1; xoffs++) {
            for(int yoffs = -1; yoffs <= 1; yoffs++) {
                if(xoffs == 0 && yoffs == 0) continue;
                put_sprite(&char_sprite, x + xoffs, y + yoffs, (style & OUTLINE_INVERTED) == 0);
            }
        }
    }

    put_sprite(&char_sprite, x, y, invert);
}

void FantaManipulator::put_string(const font_definition_t * font, const char * string, int x, int y, text_attributes_t style) {
    size_t i = 0;
    int cur_x = x;
    while(char ch = string[i]) {
        // hacky way of mapping cyrillic from utf8 to cp866 since most of our fonts are from keyrus
        if((ch & 0b11100000) == 0b11000000 && (string[i + 1] & 0b11000000) == 0b10000000) {
            if(((ch >> 2) & 7) == 0x04) {
                // U+04xx
                ch = ((ch & 0b00000011) << 6) | (string[i + 1] & 0b00111111);
                i++;
                if(ch >= 0x10 && ch <= 0x3F) {
                    ch += (0x80 - 0x10);
                }
                else if(ch >= 0x40 && ch <= 0x4F) {
                    ch += (0xE0 - 0x40);
                }
                else if(ch == 0x01) {
                    ch = 0x85;// 0xF0;
                }
                else if(ch == 0x51) {
                    ch = 0xA5;// 0xF1;
                }
                else {
                    // wasn't cyrillic, so draw as is...
                    i--;
                    ch = string[i];
                }
            }
        }
        put_glyph(font, ch, cur_x, y, style);
        cur_x += font->width;
        i++;
    }
}

void FantaManipulator::scroll(int dx, int dy) {
    if(dy != 0) {
        uint16_t * columns = (uint16_t*) buffer;
        for(int i = 0; i < width; i++) {
            if(dy > 0) {
                columns[i] <<= dy;
            } else if(dy < 0) {
                columns[i] >>= abs(dy);
            }
        }
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

void FantaManipulator::line(int x1, int y1, int x2, int y2, bool state) {
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = x1 < x2 ? 1 : -1;
  int sy = y1 < y2 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2;

  while (true) {
    plot_pixel(x1, y1, state);

    if (x1 == x2 && y1 == y2) {
      break;
    }

    int e2 = err;

    if (e2 > -dx) {
      err -= dy;
      x1 += sx;
    }

    if (e2 < dy) {
      err += dx;
      y1 += sy;
    }
  }
}

void FantaManipulator::rect(int x1, int y1, int x2, int y2, bool fill, bool state) {
    if (fill) {
        for (int i = x1; i < x2; i++) {
            for (int j = y1; j < y2+1; j++) {
                plot_pixel(i, j, state);
            }
        }
    } else {
        line(x1, y1, x2, y1, state);
        line(x2, y1, x2, y2, state);
        line(x2, y2, x1, y2, state);
        line(x1, y2, x1, y1, state);
    }
}
