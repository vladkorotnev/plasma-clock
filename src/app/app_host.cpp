#include <app/app_host.h>
#include <state.h>

static const char LOG_TAG[] = "AppHost";

AppHost::AppHost(): Composite() {
#if HAS(SPLIT_SCREEN_APPHOST)
    split_screen = new ViewMultiplexor();
    split_screen->width = HWCONF_DISPLAY_WIDTH_PX/2;
    split_screen->x_offset = -split_screen->width;
    add_composable(split_screen);
#endif

    // Must be added second, so that all HID events first go into the split screen
    main_screen = new ViewMultiplexor();
    add_composable(main_screen);
}

void AppHost::switch_to(view_id_t view, transition_type_t transition) {
#if HAS(SPLIT_SCREEN_APPHOST)
    if(state_wants_full_screen(view)) {
        ESP_LOGV(LOG_TAG, "New full-screen state %i", view);
        if(split_active) split_active = false;
#endif
        main_screen->switch_to(view, transition);
#if HAS(SPLIT_SCREEN_APPHOST)
        main_idle = (view == STATE_IDLE);
        if(!main_idle) {
            split_screen->x_offset = -split_screen->width;
            main_screen->x_offset = 0;
        }
    } else {
        ESP_LOGV(LOG_TAG, "New SPLIT-screen state %i", view);
        if(!main_idle) {
            main_screen->switch_to(STATE_IDLE, TRANSITION_SLIDE_VERTICAL_UP);
            main_idle = true;
        }
        if(!split_active) { 
            split_active = true;
            transition = TRANSITION_NONE;
            split_screen->switch_to(view, transition);
            split_screen->prepare();
        } else {
            split_screen->switch_to(view, transition);
        }
    }
#endif
}

void AppHost::add_view(Renderable * view, view_id_t id) {
#if HAS(SPLIT_SCREEN_APPHOST)
    if(state_wants_full_screen(id)) 
#endif
        main_screen->add_view(view, id);
#if HAS(SPLIT_SCREEN_APPHOST)
    else split_screen->add_view(view, id);
#endif
}

void AppHost::render(FantaManipulator* fb) {
    Composite::render(fb);
#if HAS(SPLIT_SCREEN_APPHOST)
    if((split_active && split_screen->x_offset != 0) || (!split_active && split_screen->x_offset > -split_screen->width)) {
        int x = split_screen->x_offset + split_screen->width;
        #ifdef COMPOSABLE_NO_EVENODD
            fb->line(x, 0, x, fb->get_height());
        #else
            for(int y = (even_odd ? 1 : 0); y < fb->get_height(); y += 2) {
                fb->plot_pixel(x, y, true);
            }
        #endif
    }
#endif //SPLIT_SCREEN_APPHOST
}

void AppHost::step() {
    // Handle hotkeys for entering apps quickly
    // For now, assignment is hardcoded:
    // F1: Timer
    // F2: Home (clock)
    // F3: Stopwatch

    if(!is_in_critical_state()) {
        if(hid_test_key_state(KEY_SOFT_F1) == KEYSTATE_HIT) {
            push_state(STATE_TIMER_EDITOR, TRANSITION_WIPE);
        }
        else if(hid_test_key_state(KEY_SOFT_F2) == KEYSTATE_HIT) {
            change_state(STATE_IDLE, TRANSITION_WIPE);
        }
        else if(hid_test_key_state(KEY_SOFT_F3) == KEYSTATE_HIT) {
            push_state(STATE_STOPWATCH, TRANSITION_WIPE);
        }
    }
    
#if HAS(SPLIT_SCREEN_APPHOST)
    if(split_active && split_screen->x_offset < 0) {
        split_screen->x_offset = std::min(0, split_screen->x_offset + 2);
        main_screen->x_offset = split_screen->width + split_screen->x_offset;
    }
    else if(!split_active && split_screen->x_offset > -split_screen->width) {
        split_screen->x_offset = std::max(-split_screen->width, split_screen->x_offset - 2);
        main_screen->x_offset = split_screen->width + split_screen->x_offset;
    }

    if(split_active) {
        split_screen->step();

        hid_set_lock_state(true);
        main_screen->step();
        hid_set_lock_state(false);
    } else {
        main_screen->step();
    }
#else
    main_screen->step();
#endif
}

bool AppHost::is_in_critical_state() {
    // Returns true if in a state that should not be overridable by a hotkey

    device_state_t sts = get_state();
    return 
        sts == STATE_BOOT ||
        sts == STATE_RESTART ||
        sts == STATE_ALARMING ||
#if HAS(HTTPFVU)
        sts == STATE_HTTPFVU ||
#endif
#if HAS(OTAFVU)
        sts == STATE_OTAFVU ||
#endif
        sts == STATE_NEW_YEAR;
}

bool AppHost::state_wants_full_screen(view_id_t sts) {
#if HAS(SPLIT_SCREEN_APPHOST)
    return sts == STATE_BOOT ||
     sts == STATE_IDLE ||
     sts == STATE_RESTART ||
     sts == STATE_ALARMING || 
#if HAS(HTTPFVU)
     sts == STATE_HTTPFVU || 
#endif
#if HAS(OTAFVU)
     sts == STATE_OTAFVU || 
#endif
     sts == STATE_NEW_YEAR ||
      sts == STATE_PIXEL_CAVE;
#else
    return true;
#endif
}

