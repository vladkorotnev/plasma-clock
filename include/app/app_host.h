#include <views/framework.h>
#include <state.h>

class AppHost: public ViewMultiplexor {
public:
    AppHost(): ViewMultiplexor() {}

    void step() override {
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
        
        ViewMultiplexor::step();
    }

protected:
    bool is_in_critical_state() {
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
};