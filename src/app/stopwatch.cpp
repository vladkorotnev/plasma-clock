#include <app/stopwatch.h>
#include <views/common/dropping_digits.h>
#include <service/time.h>
#include <input/keys.h>
#include <state.h>

class AppShimStopwatch::StopwatchMainScreen: public Composite {
public:
    StopwatchMainScreen(Beeper *b) {
        hourView = new DroppingDigitView(2, 0);
        minuteView = new DroppingDigitView(2, 0);
        secondView = new DroppingDigitView(2, 0, b);
        running = false;
        start_time = { 0 };
        stop_time = start_time;
        strncpy(ms_buf, ".000", 8);

        big_font = find_font(FONT_STYLE_CLOCK_FACE);
        small_font = find_font(FONT_STYLE_CLOCK_FACE_SMALL);

        int char_count = 12; // XX:XX:XX.xxx
        int text_width = 9 * big_font->width + 3 * small_font->width;
        int left_offset = HWCONF_DISPLAY_WIDTH_PX/2 - text_width/2;
        hourView->x_offset = left_offset;
        left_offset += hourView->width + big_font->width;
        minuteView->x_offset = left_offset;
        left_offset += hourView->width + big_font->width;
        secondView->x_offset = left_offset;

        add_composable(hourView);
        add_composable(minuteView);
        add_composable(secondView);
    }

    void render(FantaManipulator *fb) {
        Composite::render(fb);
        
        fb->put_glyph(big_font, ':', hourView->x_offset + hourView->width, 0);
        fb->put_glyph(big_font, ':', minuteView->x_offset + minuteView->width, 0);
        fb->put_string(small_font, ms_buf, secondView->x_offset + secondView->width, fb->get_height() - small_font->height - 1);
    }

    void step() {
        Composite::step();

        if(running) {
            stop_time = get_uptime();
        }

        if(hid_test_key_any(KEY_HEADPAT | KEY_RIGHT) == KEYSTATE_HIT) {
            running = !running;
            if(start_time == stop_time && running) {
                // We started from 0 rather than unpausing
                start_time = get_uptime();
                stop_time = get_uptime();
            }
        }
        else if(hid_test_key_any(KEY_DOWN | KEY_UP) == KEYSTATE_HIT && !running) {
            start_time = stop_time;
        }
        else if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
            pop_state(STATE_STOPWATCH, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        }

        tk_time_of_day_t dt = stop_time - start_time;
        if(dt.hour >= 100) {
            start_time.hour += 100;
            dt.hour -= 100;
        }
        hourView->value = dt.hour;
        minuteView->value = dt.minute;
        secondView->value = dt.second;

        snprintf(ms_buf, 8, ".%03d", dt.millisecond % 1000);
    }

    ~StopwatchMainScreen() {
        delete hourView;
        delete minuteView;
        delete secondView;
    }

private:
    DroppingDigitView * hourView;
    DroppingDigitView * minuteView;
    DroppingDigitView * secondView;
    bool running;
    tk_time_of_day_t start_time;
    tk_time_of_day_t stop_time;
    char ms_buf[8];
    const font_definition_t * small_font;
    const font_definition_t * big_font;
};

AppShimStopwatch::AppShimStopwatch(Beeper *b) {
    appRoot = new StopwatchMainScreen(b);
    add_composable(appRoot);
}

AppShimStopwatch::~AppShimStopwatch() {
    delete appRoot;
}