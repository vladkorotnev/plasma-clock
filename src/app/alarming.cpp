#include "app/alarming.h"
#include <rsrc/common_icons.h>
#include <service/alarm.h>
#include <stdint.h>
#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <service/time.h>
#include <service/owm/weather.h>
#include <service/prefs.h>
#include <service/power_management.h>
#include <service/localize.h>
#include <views/framework.h>
#include <views/overlays/touch_arrows_ovl.h>
#include <views/idle_screens/simple_clock.h>
#include <input/keys.h>
#include <fonts.h>
#include <state.h>

static char LOG_TAG[] = "APL_ALM";

typedef enum alarming_state {
    BLINKERING,
    HINTING_SNOOZE,
    HINTING_STOP,
    CLOCK,

    SNOOZE_HOLD_COUNTDOWN,
    SNOOZING,
    STOP_HOLD_COUNTDOWN,

    STOPPED
} alarming_state_t;

static SimpleClock *clockView = nullptr;
static NewSequencer *seq = nullptr;
static TouchArrowOverlay *arrows = nullptr;
static TickType_t snoozeUntil;
static alarming_state_t state;
static uint8_t framecount = 0;
static bool clock_inverting = false;
static uint8_t snooze_hold_remain = 0;
static int snooze_minutes = 0;
static bool is_snoozing = false;
static const alarm_setting_t * alarm_setting;
static TickType_t startedAt;
static TickType_t maxDur;

void app_alarming_prepare(NewSequencer* s) {
    startedAt = xTaskGetTickCount();
    maxDur = pdMS_TO_TICKS( prefs_get_int(PREFS_KEY_ALARM_MAX_DURATION_MINUTES) * 60000 );

    if(seq) {
        seq->stop_sequence();
    }
    seq = s;

    if(!arrows) arrows = new TouchArrowOverlay();
    if(!clockView) clockView = new SimpleClock();
    arrows->prepare();
    arrows->active = true;
    clockView->prepare();
    framecount = 0;
    snooze_minutes = prefs_get_int(PREFS_KEY_ALARM_SNOOZE_MINUTES);

    alarm_setting = get_triggered_alarm();
    if(alarm_setting) {
        auto melody = melody_from_no(alarm_setting->melody_no);
        seq->play_sequence(melody, SEQUENCER_REPEAT_INDEFINITELY);
    }
    power_mgmt_pause();
    state = BLINKERING;
}

void app_alarming_draw(FantaManipulator* fb) {
    framecount++;
    fb->clear();

    switch(state) {
        case BLINKERING:
            snooze_hold_remain = fb->get_width();
            // Draw the animation of a buzzing alarm clock
            if(framecount < 120) {
                // Roughly 2 seconds idle time
                fb->put_sprite(&alarm_icns, fb->get_width() / 2 - alarm_icns.width / 2, 0);
            } else {
                int offset = (framecount % 10) - 5;
                fb->put_sprite(&alarm_icns, fb->get_width() / 2 - alarm_icns.width / 2 + offset, 0);
                if(framecount % 10 == 0) {
                    clock_inverting = !clock_inverting;
                }
                if(clock_inverting) {
                    fb->invert();
                }

                if(framecount == 255) {
                    #if HAS(TOUCH_PLANE) || HAS(KEYPAD)
                    arrows->left = (snooze_minutes > 0);
                    arrows->top = false;
                    arrows->bottom = false;
                    arrows->right = (snooze_minutes == 0);
                    framecount = 0;
                    state = (snooze_minutes > 0) ? HINTING_SNOOZE : HINTING_STOP;
                    #else
                    framecount = 0;
                    state = CLOCK;
                    #endif
                }
            } 
            break;

        case HINTING_SNOOZE:
            {
                snooze_hold_remain = fb->get_width();
                static const char * snooze_str = localized_string("SNOOZE");
                fb->put_string(&keyrus0816_font, snooze_str, 12, 0);
                arrows->render(fb);
                if(framecount >= 180) {
                    arrows->left = false;
                    arrows->right = true;
                    arrows->top = false;
                    arrows->bottom = false;
                    framecount = 0;
                    state = HINTING_STOP;
                }
            }
            break;

        case HINTING_STOP:
            {
                snooze_hold_remain = fb->get_width();
                static const char * stop_str = localized_string("STOP");
                fb->put_string(&keyrus0816_font, stop_str, fb->get_width() - 12 - measure_string_width(&keyrus0816_font, stop_str), 0);
                arrows->render(fb);
                if(framecount >= 180) {
                    framecount = 0;
                    state = CLOCK;
                }
            }
            break;

        case CLOCK:
            snooze_hold_remain = fb->get_width();
            clockView->render(fb);
            if(framecount % 30 == 0) {
                clock_inverting = !clock_inverting;
            }
            if(clock_inverting) {
                fb->invert();
            }
            if(framecount >= 240) {
                framecount = 0;
                state = BLINKERING;
            }
            break;

        case SNOOZE_HOLD_COUNTDOWN:
            {
                static const char * hold_str = localized_string("HOLD");
                fb->put_string(&keyrus0816_font, hold_str, 12, 0);
                arrows->render(fb);
                if(snooze_hold_remain > 0) snooze_hold_remain -= 1;
                FantaManipulator *holdProgress = fb->slice(snooze_hold_remain, fb->get_width() - snooze_hold_remain);
                holdProgress->invert();
                delete holdProgress;
            }
            break;

        case STOP_HOLD_COUNTDOWN:
            {
                static const char * hold_str = localized_string("HOLD");
                fb->put_string(&keyrus0816_font, hold_str, fb->get_width() - 12 - measure_string_width(&keyrus0816_font, hold_str), 0);
                arrows->render(fb);
                if(snooze_hold_remain > 0 && framecount % 2 == 0) snooze_hold_remain -= 1;
                FantaManipulator *holdProgress = fb->slice(0,  fb->get_width() - snooze_hold_remain);
                holdProgress->invert();
                delete holdProgress;
            }
            break;

        case SNOOZING:
            {
                snooze_hold_remain = fb->get_width();
                clockView->render(fb);
                arrows->render(fb);
            }
            break;

        default: break;
    }
}

void begin_snoozing() {
    state = SNOOZING;
    is_snoozing = true;
    snoozeUntil = xTaskGetTickCount() + pdMS_TO_TICKS(snooze_minutes * 60 * 1000);
    arrows->right = true;
    arrows->top = false;
    arrows->bottom = false;
    arrows->left = false;
    seq->stop_sequence();
}

void stop_alarm() {
    state = STOPPED;
    clear_triggered_alarm();
    seq->stop_sequence();
    power_mgmt_resume();
    pop_state(STATE_ALARMING, TRANSITION_WIPE);
}

void app_alarming_process() {
    if(state == CLOCK || state == SNOOZING) clockView->step();

    switch(state) {
        case BLINKERING:
        case HINTING_SNOOZE:
        case HINTING_STOP:
        case CLOCK:
            {
                if(hid_test_key_any(KEY_LEFT | KEY_UP | KEY_DOWN) && snooze_minutes > 0) {
                    arrows->left = true;
                    arrows->top = false;
                    arrows->bottom = false;
                    arrows->right = false;
                    state = SNOOZE_HOLD_COUNTDOWN;
                    return;
                }
                else if(hid_test_key_state(KEY_RIGHT)) {
                    arrows->left = false;
                    arrows->top = false;
                    arrows->bottom = false;
                    arrows->right = true;
                    state = STOP_HOLD_COUNTDOWN;
                    return;
                }
                else if(hid_test_key_state(KEY_HEADPAT)) {
                    arrows->left = false;
                    arrows->top = true;
                    arrows->bottom = false;
                    arrows->right = false;
                    state = (snooze_minutes > 0) ? SNOOZE_HOLD_COUNTDOWN : STOP_HOLD_COUNTDOWN;
                }

                if(maxDur > 0) {
                    // there is a max set limit
                    if(xTaskGetTickCount() - startedAt > maxDur) {
                        stop_alarm();
                        return;
                    }
                }
            }
        break;

        case SNOOZE_HOLD_COUNTDOWN:
            if(!hid_test_key_any(KEY_LEFT | KEY_UP | KEY_DOWN | KEY_HEADPAT) && snooze_hold_remain > 0) {
                state = HINTING_SNOOZE; // fail to hold
                framecount = 0;
            } else if(snooze_hold_remain == 0) {
                begin_snoozing();
            }
        break;

        case STOP_HOLD_COUNTDOWN:
            if(!hid_test_key_any(KEY_RIGHT | KEY_HEADPAT) && snooze_hold_remain > 0) {
                state = is_snoozing ? SNOOZING : HINTING_STOP; // fail to hold
                framecount = 0;
            } else if(snooze_hold_remain == 0) {
                stop_alarm();
            }
        break;

        case SNOOZING:
            if(hid_test_key_state(KEY_RIGHT)) {
                state = STOP_HOLD_COUNTDOWN;
                return;
            } else if(xTaskGetTickCount() >= snoozeUntil) {
                is_snoozing = false;
                startedAt = xTaskGetTickCount();
                state = BLINKERING;
                if(alarm_setting) {
                    auto melody = melody_from_no(alarm_setting->melody_no);
                    seq->play_sequence(melody, SEQUENCER_REPEAT_INDEFINITELY);
                }
            }
        break;

        default: break;
    }
}