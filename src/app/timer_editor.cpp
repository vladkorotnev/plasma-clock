#include <app/timer_editor.h>
#include <state.h>
#include <views/menu/time_setting.h>
#include <service/prefs.h>
#include <service/localize.h>
#include <input/keys.h>

class AppShimTimerEditor::TimerEditorMainScreen: public Composite {
public:
    static void xTimerCallback(TimerHandle_t xTimer) {
        TimerEditorMainScreen * that = static_cast<TimerEditorMainScreen *>(pvTimerGetTimerID( xTimer ));
        that->tick();
    }

    TimerEditorMainScreen(Beeper *b, NewSequencer *s, std::function<void(bool, Renderable *)> _activation) {
        hourView = new DroppingDigitView(2, 0, b);
        minuteView = new DroppingDigitView(2, 0, b);
        secondView = new DroppingDigitView(2, 0, b);

        int char_count = 10; // XX:XX:XX []
        int text_width = char_count * xnu_font.width;
        int left_offset = HWCONF_DISPLAY_WIDTH_PX/2 - text_width/2;
        hourView->x_offset = left_offset;
        left_offset += hourView->width + xnu_font.width;
        minuteView->x_offset = left_offset;
        left_offset += hourView->width + xnu_font.width;
        secondView->x_offset = left_offset;

        add_composable(hourView);
        add_composable(minuteView);
        add_composable(secondView);

        beeper = b;
        sequencer = s;
        activation = _activation;
        if(beeper != nullptr) {
            melodySelector = new MenuMelodySelectorPreferenceView(sequencer, localized_string("Melody"), PREFS_KEY_TIMER_MELODY, [_activation](bool isActive, Renderable * that) {
                if(!isActive) {
                    _activation(false, that);
                }
            });
        }
        load_prefs();
        cursorTimer = 0;
        isRunning = false;
        mustRing = false;
        blinkState = false;

        hTimer = xTimerCreate(
            "TMR",
            pdMS_TO_TICKS(1000),
            true,
            this,
            xTimerCallback
        );
    }

    ~TimerEditorMainScreen() {
        if(melodySelector != nullptr) {
            delete melodySelector;
        }

        delete hourView;
        delete minuteView;
        delete secondView;
    }

    void render(FantaManipulator *fb) {
        cursorTimer++;
        if(cursorTimer % 30 == 0) {
            isShowingCursor = !isShowingCursor;
        }

        int cursor_offset = 0;

        if(cursorPosition == CursorPosition::HOUR) cursor_offset = hourView->x_offset;
        if(cursorPosition == CursorPosition::MINUTE) cursor_offset = minuteView->x_offset;
        if(cursorPosition == CursorPosition::SECOND) cursor_offset = secondView->x_offset;

        Composite::render(fb);
        
        fb->put_glyph(&xnu_font, ':', hourView->x_offset + hourView->width, 0);
        fb->put_glyph(&xnu_font, ':', minuteView->x_offset + minuteView->width, 0);

        int left_offset = secondView->x_offset + secondView->width + xnu_font.width;

        if(isRunning) fb->rect(left_offset-1, 0, left_offset + 17, 15, true);
        fb->put_glyph(&keyrus0816_font, 0x10, left_offset, 0, isRunning ? TEXT_INVERTED : TEXT_NORMAL);
        if(cursorPosition == CursorPosition::PLAY_PAUSE) cursor_offset = left_offset;

        if(isShowingCursor) {
            fb->rect(cursor_offset - 2, 0, cursor_offset + 2 * xnu_font.width + 1, 15, false);
        }

        if(cursorTimer % 60 == 0 && sequencer->is_sequencing()) {
            blinkState = !blinkState;
        }

        if(blinkState) fb->invert();

        if(cursorTimer >= 179) cursorTimer = 0;
    }

    void step() {
        if(mustRing) {
            sequencer->play_sequence(melody_from_no(prefs_get_int(PREFS_KEY_TIMER_MELODY)), SEQUENCER_REPEAT_INDEFINITELY); 
            mustRing = false;
        }
        
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

        if(hid_test_key_any() && sequencer && sequencer->is_sequencing()) { 
            sequencer->stop_sequence();
            secondView->sound = true;
            minuteView->sound = true;
            hourView->sound = true;
            blinkState = false;
        }

        Composite::step();
    }

    void tick() {
        if(!isRunning) return;

        if(hour == 0 && minute == 0 && second == 0) {
            isRunning = false;
            push_state(STATE_TIMER_EDITOR); // in case we are on a different screen
            if(sequencer) { 
                secondView->sound = false;
                minuteView->sound = false;
                hourView->sound = false;
                blinkState = true;
                mustRing = true; // rather than increasing timer stack size just use a flag to kick off the melody in a bigger thread
            }
            load_prefs();
        } else {
            add_sec(false);
        }
    }

private:
    std::function<void(bool, Renderable *)> activation;
    Beeper *beeper;
    NewSequencer *sequencer;
    MenuMelodySelectorPreferenceView * melodySelector;
    enum CursorPosition {
        HOUR, MINUTE, SECOND, PLAY_PAUSE
    };
    bool isRunning;
    int hour, minute, second;
    DroppingDigitView * hourView;
    DroppingDigitView * minuteView;
    DroppingDigitView * secondView;
    uint8_t cursorTimer;
    CursorPosition cursorPosition;
    bool isShowingCursor;
    bool mustRing;
    bool blinkState;
    TimerHandle_t hTimer;

    void load_prefs() {
        int saved_time = prefs_get_int(PREFS_KEY_TIMER_TIME_SECONDS);
        hour = saved_time / 3600;
        minute = (saved_time % 3600) / 60;
        second = saved_time % 60;
        cursorPosition = saved_time == 0 ? CursorPosition::SECOND : CursorPosition::PLAY_PAUSE;
        hourView->value = hour;
        minuteView->value = minute;
        secondView->value = second;
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
        hourView->value = hour;
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
        minuteView->value = minute;
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
        secondView->value = second;
    }
    void start_stop() {
        isRunning = !isRunning;
        if(isRunning) {
            save_prefs();
            cursorPosition = CursorPosition::PLAY_PAUSE;
            xTimerStart(hTimer, 0);
            secondView->sound = true;
            minuteView->sound = false;
            hourView->sound = false;
        } else {
            xTimerStop(hTimer, 0);
            // save time halfway through
            save_prefs();
            secondView->sound = true;
            minuteView->sound = true;
            hourView->sound = true;
        }
    }
};

AppShimTimerEditor::AppShimTimerEditor(Beeper * b, NewSequencer * s): ProtoShimNavigationStack(
    new TimerEditorMainScreen(b, s, [this](bool isActive, Renderable * dst) {
        if(isActive) push_renderable(dst);
        else pop_renderable();
    })
) {
    appRoot = (TimerEditorMainScreen*) _root;
}

AppShimTimerEditor::~AppShimTimerEditor() {
    delete appRoot;
}
