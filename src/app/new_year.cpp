#include <app/new_year.h>
#include <state.h>
#include <service/localize.h>
#include <esp32-hal-log.h>
#include "../sound/melodies/abba.h"
#include <fonts.h>
#include <Arduino.h>
#include <algorithm>

static char LOG_TAG[] = "APL_NY";
const unsigned bg_left_margin = 28;
static const uint8_t bg_image[] = {
    // 'ny_bg', 16x100px
    0x00,0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0xa0, 0x00, 0xd2, 0x00, 0xed, 
    0xa0,0xde, 0xd0, 0xbd, 0xba, 0xdb, 0x7f, 0xed, 0xba, 0xdb, 0xd0, 0xbd, 0xa0, 0xde, 0x00, 0xed, 
    0x00,0xd2, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0xe0, 
    0x00,0xe0, 0x00, 0xc0, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 
    0x00,0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 
    0x00,0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 
    0x00,0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 
    0x00,0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 
    0x00,0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 
    0x00,0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xf0, 
    0x00,0xf0, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xff, 0x00, 0xff, 
    0x00,0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 
    0x00,0xff, 0x00, 0xff, 0x80, 0xff, 0x80, 0xff
};

static inline void _print_mem(const char * descr) {
    ESP_LOGI(LOG_TAG, " === %s === ", descr);
    ESP_LOGI(LOG_TAG, "HEAP: %d free of %d (%d minimum)", ESP.getFreeHeap(), ESP.getHeapSize(), ESP.getMinFreeHeap());
#ifdef BOARD_HAS_PSRAM
    ESP_LOGI(LOG_TAG, "PSRAM: %d free of %d (%d minimum)", ESP.getFreePsram(), ESP.getPsramSize(), ESP.getMinFreePsram());
#endif
}

NewYearAppShim::NewYearAppShim(Beeper *b, NewSequencer *s, Yukkuri *y): 
    sequencer(s),
    balabol(y),
    beeper(b),
    fireworks { nullptr },
    snow { nullptr },
    utterance_localized { nullptr },
    allocated { false },
    Composite() 
{
    wants_clear_surface = true;
}

void NewYearAppShim::allocate_all() {
    if(allocated) return;

    _print_mem("BEFORE ALLOC");
    fireworks = new FireworksOverlay(beeper);
    fireworks->intense = false;
    fireworks->sound = false;
    fireworks->min_delay = pdMS_TO_TICKS(5000); // hold em off for now
    fireworks->max_delay = pdMS_TO_TICKS(10000);
    fireworks->set_active(true);

    snow = new RainOverlay();
    snow->set_windspeed(0, false);
    snow->set_gravity(1, false);
    snow->set_intensity(6);
    snow->set_speed_divisor(3);

    add_subrenderable(fireworks);
    add_subrenderable(snow);

    switch(active_tts_language()) {
        case TTS_LANG_RU:
            utterance_localized = "_su/no'vimu/go-da'-nn,";
            break;

        case TTS_LANG_JA:
            utterance_localized = "akemasite'+omedeto'-,";
            break;

        case TTS_LANG_EN:
        default:
            utterance_localized = "happi-/nyu'-/iea-,";
            break;
    }

    tk_date_t today = get_current_date();
    digits[0] = '0' + (today.year / 1000);
    digits[1] = '0' + (today.year % 1000) / 100;
    digits[2] = '0' + (today.year % 100) / 10;
    digits[3] = '0' + (today.year % 10);

    // memo: each digit has 200ms to come in and 27ms delay inbetween

    _print_mem("AFTER ALLOC");
}

void NewYearAppShim::prepare() {
    allocate_all();
    Composite::prepare();
    start_time = xTaskGetTickCount();
    phase = INTRO;
    sequencer->play_sequence(&abba);
}

void NewYearAppShim::render(FantaManipulator* fb) {
    Composite::render(fb);
    TickType_t now = xTaskGetTickCount();
    int digit_x = fb->get_width() / 2 - xnu_font.width * 2;

    for(int i = 0; i < 4; i++) {
        int digit_y = 0;
        if(phase == INTRO) {
            int intro_elapsed = pdTICKS_TO_MS(now - start_time);
            int progress_in_digit = std::max(0, std::min(intro_elapsed - (227 * i), 200));
            digit_y = fb->get_height() - (fb->get_height() * progress_in_digit / 200);
        }
        fb->put_glyph(&xnu_font, digits[i], digit_x, digit_y, TEXT_NO_BACKGROUND);
        digit_x += xnu_font.width;
    }

    fb->put_fanta((fanta_buffer_t) bg_image, 0, 0, 100, 16, (fanta_buffer_t) bg_image);
}

void NewYearAppShim::step() {
    Composite::step();
    TickType_t now = xTaskGetTickCount();

    switch(phase) {
        case INTRO:
            if(now - start_time >= abba_melody_ascend_time_ms) {
                fireworks->intense = true;
                fireworks->min_delay = pdMS_TO_TICKS(33);
                fireworks->max_delay = pdMS_TO_TICKS(1000);
                phase = INTENSE;
            }
            break;
        case INTENSE:
            if(!sequencer->is_sequencing()) {
                phase = ANNOUNCE;
                balabol->speak(utterance_localized, [this](bool) {
                    // don't care about return code, since it returns false when built without AquesTalk library
                    fireworks->sound = true;
                    phase = FIREWORKS;
                    start_time = xTaskGetTickCount();
                });
            }
            break;

        case ANNOUNCE:
            // Transition handled in utterance callback
            break;

        case FIREWORKS:
            if(now - start_time >= pdMS_TO_TICKS(5000)) {
                phase = CHILLOUT;
                start_time = now;
                fireworks->intense = false;
                fireworks->min_delay = pdMS_TO_TICKS(300);
                fireworks->max_delay = pdMS_TO_TICKS(2000);
            }
            break;
        case CHILLOUT:
            if(now - start_time >= pdMS_TO_TICKS(5000)) {
                phase = FIN;
                ESP_LOGI(LOG_TAG, "Finita la comedia");
                pop_state(STATE_NEW_YEAR, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
            }
            break;
    }
}
