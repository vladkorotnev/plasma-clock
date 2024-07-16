#include <app/timer_editor.h>
#include <state.h>
#include <views/menu/time_setting.h>
#include <service/prefs.h>
#include <input/keys.h>


class AppShimTimerEditor::TimerEditorMainScreen: public Renderable, DroppingDigits {
public:
    static void xTimerCallback(TimerHandle_t xTimer) {
        TimerEditorMainScreen * that = static_cast<TimerEditorMainScreen *>(pvTimerGetTimerID( xTimer ));
        that->tick();
    }

    TimerEditorMainScreen(Beeper *b, std::function<void(bool, Renderable *)> _activation): DroppingDigits() {
        beeper = b;
        activation = _activation;
        if(beeper != nullptr) {
            sequencer = new BeepSequencer(beeper);
            melodySelector = new MenuMelodySelectorPreferenceView(beeper, "Timer Melody", PREFS_KEY_TIMER_MELODY, [_activation](bool isActive, Renderable * that) {
                if(!isActive) {
                    _activation(false, that);
                }
            });
        }
        load_prefs();
        animationPhase = -1;
        cursorTimer = 0;
        isRunning = false;

        hTimer = xTimerCreate(
            "TMR",
            pdMS_TO_TICKS(1000),
            true,
            this,
            xTimerCallback
        );
    }

    ~TimerEditorMainScreen() {
        if(sequencer != nullptr) {
            delete sequencer;
        }
        if(melodySelector != nullptr) {
            delete melodySelector;
        }
    }

    void render(FantaManipulator *fb) {
        if(animationPhase > -1) {
            animationPhase++;
            cursorTimer = 0;
            isShowingCursor = false;
            if(animationPhase == 16) {
                animationPhase = -1;
                isShowingCursor = true;
                prevHour = hour;
                prevMinute = minute;
                prevSecond = second;
            }
        } else {
            prevHour = hour;
            prevMinute = minute;
            prevSecond = second;
        }

        cursorTimer++;
        if(cursorTimer == 30) {
            cursorTimer = 0;
            isShowingCursor = !isShowingCursor;
        }

        int char_count = 10; // XX:XX:XX []
        int text_width = char_count * font->width;
        int left_offset = fb->get_width()/2 - text_width/2;
        int cursor_offset = 0;

        draw_dropping_number(fb, prevHour, hour, animationPhase, left_offset);
        if(cursorPosition == CursorPosition::HOUR) cursor_offset = left_offset;
        left_offset += 2 * font->width;
        
        fb->put_glyph(font, ':', left_offset, 0);
        left_offset += font->width;

        draw_dropping_number(fb, prevMinute, minute, animationPhase, left_offset);
        if(cursorPosition == CursorPosition::MINUTE) cursor_offset = left_offset;
        left_offset += 2 * font->width;

        fb->put_glyph(font, ':', left_offset, 0);
        left_offset += font->width;

        draw_dropping_number(fb, prevSecond, second, animationPhase, left_offset);
        if(cursorPosition == CursorPosition::SECOND) cursor_offset = left_offset;
        left_offset += 3*font->width;

        if(isRunning) fb->rect(left_offset-1, 0, left_offset + 17, 15, true);
        fb->put_glyph(&keyrus0816_font, 0x10, left_offset, 0, isRunning);
        if(cursorPosition == CursorPosition::PLAY_PAUSE) cursor_offset = left_offset;

        if(isShowingCursor) {
            fb->rect(cursor_offset - 2, 0, cursor_offset + 2 * font->width + 1, 15, false);
        }
    }

    void step() {
        if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
            if(cursorPosition == CursorPosition::HOUR || isRunning) {
                // too much to left, leave
                pop_state(STATE_TIMER_EDITOR, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
            } else {
                cursorPosition = (CursorPosition) (((int) cursorPosition) - 1);
            }
        } 
        else if(hid_test_key_state_repetition(KEY_DOWN)) {
            switch (cursorPosition) {
                case CursorPosition::HOUR: add_hr(false); break;
                case CursorPosition::MINUTE: add_min(false); break;
                case CursorPosition::SECOND: add_sec(false); break;
                case CursorPosition::PLAY_PAUSE: start_stop(); break;
            }
        }
        else if(hid_test_key_state_repetition(KEY_UP)) {
            switch (cursorPosition) {
                case CursorPosition::HOUR: add_hr(true); break;
                case CursorPosition::MINUTE: add_min(true); break;
                case CursorPosition::SECOND: add_sec(true); break;
                case CursorPosition::PLAY_PAUSE: start_stop(); break;
            }
        }
        else if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            if(cursorPosition == CursorPosition::PLAY_PAUSE && melodySelector != nullptr) {
                // too much to right, show melody picker
                melodySelector->isActive = true;
                activation(true, melodySelector);
            } else {
                cursorPosition = (CursorPosition) (((int) cursorPosition) + 1);
            }
        }

        if(hid_test_key_any() && sequencer) sequencer->stop_sequence();

        if(animationPhase == 8) {
            beeper->beep_blocking(CHANNEL_AMBIANCE, 100, 10);
        }
    }

    void tick() {
        if(!isRunning) return;

        if(hour == 0 && minute == 0 && second == 0) {
            isRunning = false;
            push_state(STATE_TIMER_EDITOR); // in case we are on a different screen
            if(sequencer) sequencer->play_sequence(melody_from_no(prefs_get_int(PREFS_KEY_TIMER_MELODY)), CHANNEL_ALARM, SEQUENCER_REPEAT_INDEFINITELY);
            load_prefs();
        } else {
            add_sec(false);
        }
    }

private:
    std::function<void(bool, Renderable *)> activation;
    Beeper *beeper;
    BeepSequencer *sequencer;
    MenuMelodySelectorPreferenceView * melodySelector;
    enum CursorPosition {
        HOUR, MINUTE, SECOND, PLAY_PAUSE
    };
    bool isRunning;
    int hour, minute, second;
    int prevHour;
    int prevMinute;
    int prevSecond;
    int animationPhase;
    uint8_t cursorTimer;
    CursorPosition cursorPosition;
    bool isShowingCursor;
    TimerHandle_t hTimer;

    void load_prefs() {
        int saved_time = prefs_get_int(PREFS_KEY_TIMER_TIME_SECONDS);
        hour = saved_time / 3600;
        minute = (saved_time % 3600) / 60;
        second = saved_time % 60;
        cursorPosition = saved_time == 0 ? CursorPosition::SECOND : CursorPosition::PLAY_PAUSE;
    }

    void save_prefs() {
        int total_seconds = hour * 3600 + minute * 60 + second;
        if(total_seconds == 0) return;
        prefs_set_int(PREFS_KEY_TIMER_TIME_SECONDS, total_seconds);
    }

    void add_hr(bool increment) {
        if(increment) {
            hour += 1;
            if(hour > 23) hour = 0;
        } else {
            hour -= 1;
            if(hour < 0) hour = 23;
        }

        if(animationPhase == -1) animationPhase = 0;
    }
    void add_min(bool increment) {
        if(increment) {
            minute += 1;
            if(minute > 59) {
                minute = 0;
                add_hr(true);
            }
        } else {
            minute -= 1;
            if(minute < 0) {
                minute = 59;
                add_hr(false);
            }
        }
        if(animationPhase == -1) animationPhase = 0;
    }
    void add_sec(bool increment) {
        if(increment) {
            second += 1;
            if(second > 59) {
                second = 0;
                add_min(true);
            }
        } else {
            second -= 1;
            if(second < 0) {
                second = 59;
                add_min(false);
            }
        }
        if(animationPhase == -1) animationPhase = 0;
    }
    void start_stop() {
        isRunning = !isRunning;
        if(isRunning) {
            save_prefs();
            cursorPosition = CursorPosition::PLAY_PAUSE;
            xTimerStart(hTimer, 0);
        } else {
            xTimerStop(hTimer, 0);
            // save time halfway through
            save_prefs();
        }
    }
};

AppShimTimerEditor::AppShimTimerEditor(Beeper * b): ProtoShimNavigationStack(
    new TimerEditorMainScreen(b, [this](bool isActive, Renderable * dst) {
        if(isActive) push_renderable(dst);
        else pop_renderable();
    })
) {
    appRoot = (TimerEditorMainScreen*) _root;
}

AppShimTimerEditor::~AppShimTimerEditor() {
    delete appRoot;
}
