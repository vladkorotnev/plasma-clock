#include <console.h>
#include <string.h>
#include <Arduino.h>

static const char CURSOR_OFF = ' ';
static char LOG_TAG[] = "CONS";

extern "C" void ConsoleTaskFunction( void * pvParameter );

void ConsoleTaskFunction( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "Running task");
    Console * that = static_cast<Console*> ( pvParameter );


    while(1) {
        that->task();
    }
}


Console::Console(const font_definition_t * f, PlasmaDisplayFramebuffer * fb) {
    disp = fb;
    font = f;
    cursor_x = 0;
    cursor_y = 0;
    cursor_enable = true;
    cursor_state = false;
    hQueue = xQueueCreate(10, sizeof(char *));

    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        ConsoleTaskFunction,
        "CONS",
        4096,
        this,
        10,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

Console::~Console() {
    if(hTask != NULL) {
        vTaskDelete(hTask);
        hTask = NULL;
    }
}

void Console::task() {
    char * next_line = nullptr;
    if(xQueueReceive(hQueue, &next_line, pdMS_TO_TICKS( 500 )) == pdTRUE) {
        // Output next line
        if(cursor_enable && cursor_state) {
            disp->put_glyph(font, CURSOR_OFF, cursor_x, cursor_y);
        }
        for(int i = 0; i < strlen(next_line); i++) {
            char ch = next_line[i];

            switch(ch) {
                case '\n':
                    cursor_newline();
                case '\r':
                    cursor_x = 0;
                    break;

                // TODO: tabs etc
                default:
                    if(cursor_x + font->width > disp->width) {
                        cursor_newline();
                        cursor_x = 0;
                    }
                    disp->put_glyph(font, ch, cursor_x, cursor_y);
                    cursor_x += font->width;
                    break;
            }
        }
        free(next_line);

        cursor_state = false;
    }

    if(cursor_enable) {
        cursor_state = !cursor_state;
        disp->put_glyph(font, cursor_state ? font->cursor_character : CURSOR_OFF, cursor_x, cursor_y);
    }
}

void Console::clear() {
    disp->clear();
    cursor_x = 0;
    cursor_y = 0;
}

void Console::set_cursor(bool enable) {
    if(enable && !cursor_enable) {
        cursor_state = true;
        disp->put_glyph(font, font->cursor_character, cursor_x, cursor_y);
    } else if(!enable && cursor_enable) {
        disp->put_glyph(font, CURSOR_OFF, cursor_x, cursor_y);
    }
    cursor_enable = enable;
}

void Console::cursor_newline() {
    if(cursor_y + font->height * 2 > disp->height) {
        // Next line won't fit, so scroll current content above and keep Y same
        disp->scroll(0, -font->height);
    } else {
        cursor_y += font->height;
    }
}

void Console::print(const char * text, ...) {
    va_list arg1, arg2;

    va_start(arg1, text);

    // Find the total length
    va_copy(arg2, arg1);
    size_t size = vsnprintf(NULL, 0, text, arg2)  + 2;
    va_end(arg2);

    char *szBuff = (char*) malloc(size);
    vsnprintf(&szBuff[1], size, text, arg1);
    szBuff[0] = '\n';

    va_end(arg1);

    ESP_LOGV(LOG_TAG, "%s", &szBuff[1]);

    xQueueSend(hQueue, (void*) &szBuff, portMAX_DELAY);
}

void Console::write(char ch) {
    char * msg = (char*) malloc(2);
    msg[0] = ch;
    msg[1] = 0x0;
    xQueueSend(hQueue, (void*) &msg, portMAX_DELAY);
}

void Console::set_font(const font_definition_t* f) {
    font = f;
}