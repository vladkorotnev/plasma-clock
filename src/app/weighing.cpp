#include <app/weighing.h>
#include <service/balance_board.h>
#include <state.h>
#include <service/prefs.h>

#if HAS(BALANCE_BOARD_INTEGRATION)

class AppShimWeighing::WeighingView: public Renderable {
public:
    WeighingView(SensorPool*s): 
        sensors(s),
        font(&keyrus0816_font),
        value_str { 0 },
        value_len { 0 },
        disp_value { 0 }
    {}

    void render(FantaManipulator *fb) {
        int text_width = value_len * font->width;
        int left_offset = fb->get_width() / 2 - text_width / 2;
        fb->put_string(font, value_str, left_offset, 0);
    }

    void step() {
        if(sensors->exists(SENSOR_ID_BALANCE_BOARD_MILLIKILOS)) {
            if(sensor_info_t * info = sensors->get_info(SENSOR_ID_BALANCE_BOARD_MILLIKILOS)) {
                int value = info->last_result / 10;
                snprintf(value_str, 12, "%c %d.%d kg %c", value == 0 ? 0x10 : ' ', value / 10, abs(value % 10), value == 0 ? 0x11 : ' ');
                value_len = strlen(value_str);
            }
        }
    }

private:
    SensorPool *sensors;
    const font_definition_t * font;
    int disp_value;
    char value_str[12];
    int value_len;
};


AppShimWeighing::AppShimWeighing(SensorPool* s) {
    carousel = new ViewMultiplexor();
    sensors = s;
    lastActivity = xTaskGetTickCount();

    carousel->add_view(new MenuInfoItemView("Disconnected", "\x1A to connect"), NEED_CONNECT);
    carousel->add_view(new MenuInfoItemView("Scanning...", "Press the SYNC button on the Balance Board now"), WAIT_CONNECT);
    carousel->add_view(new WeighingView(s), WEIGHING);

    add_subrenderable(carousel);

    balance_board_start(s);
}

AppShimWeighing::~AppShimWeighing() {
    delete carousel;
}

void AppShimWeighing::prepare() {
    Composite::prepare();
    lastActivity = xTaskGetTickCount();
    update_state(TRANSITION_NONE);
}

void AppShimWeighing::render(FantaManipulator* fb) {
    fb->clear();
    Composite::render(fb);
}

void AppShimWeighing::step() {
    Composite::step();
    update_state(TRANSITION_WIPE);
    balance_board_state_t sts = balance_board_state();
    if(sts == BB_IDLE) {
        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            balance_board_scan(true);
        }
    } else if(sts == BB_SCANNING) {
        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            balance_board_scan(false);
        }
    } else if(sts == BB_CONNECTED) {
        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            balance_board_zero();
        } else if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HOLDING) {
            balance_board_scan(true);
        }
    }

    if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
        if(sts == BB_SCANNING) balance_board_scan(false);
        pop_state(STATE_WEIGHING, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
    }

    TickType_t now = xTaskGetTickCount();
    if(sensor_info_t * activity = sensors->get_info(SENSOR_ID_BALANCE_BOARD_STARTLED)) {
        if(activity->last_result) {
            lastActivity = now;
        }
    }

    if(now - lastActivity > pdMS_TO_TICKS(25000) && sts == BB_CONNECTED) {
        pop_state(STATE_WEIGHING, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    }
}

void AppShimWeighing::update_state(transition_type_t t) {
    switch(balance_board_state()) {
        case BB_IDLE:
            carousel->switch_to(NEED_CONNECT, t);
            break;
        case BB_SCANNING:
            carousel->switch_to(WAIT_CONNECT, t);
            break;
        case BB_CONNECTED:
            carousel->switch_to(WEIGHING, t);
            break;
    }
}

#endif
