#include <app/menu.h>
#include <service/prefs.h>
#include <state.h>
#include <views/menu/menu.h>
#include <network/netmgr.h>
#include <rsrc/common_icons.h>

AppShimMenu::AppShimMenu() {
    scroll_guidance = new TouchArrowOverlay();
    scroll_guidance->top = true;
    scroll_guidance->bottom = true;
    scroll_guidance->right = true;

    // NB: This leaks a lot of objects on purpose.
    // This object is not supposed to be instantiated more than once in the firmware.

    static ListView * clock_menu = new ListView();
    clock_menu->add_view(new MenuBooleanSettingView("Blink separators", PREFS_KEY_BLINK_SEPARATORS));
    clock_menu->add_view(new MenuBooleanSettingView("Tick sound", PREFS_KEY_TICKING_SOUND));
    clock_menu->add_view(new MenuBooleanSettingView("Ticking only when screen on", PREFS_KEY_NO_SOUND_WHEN_OFF));
    clock_menu->add_view(new MenuBooleanSettingView("Hourly chime", PREFS_KEY_HOURLY_CHIME_ON));
    // TODO: Melody first of the day
    // TODO: Melody all others
    // TODO: int From, int To
    // TODO?: NTP server
    // TODO: NTP interval
    // TODO: Timezone
    // TODO: Time Set

    static ListView * system_info = new ListView();
    system_info->add_view(new MenuInfoItemView("OS Type", PRODUCT_NAME));
    system_info->add_view(new MenuInfoItemView("OS Version", PRODUCT_VERSION));
    system_info->add_view(new MenuInfoItemView("WiFi Name", NetworkManager::network_name()));
    char buf_ip[17] = { 0 };
    strncpy(buf_ip, NetworkManager::current_ip().c_str(), 16);
    system_info->add_view(new MenuInfoItemView("WiFi IP", buf_ip));

    static const uint8_t status_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x0f, 0xf0, 0x11, 0xf8, 0x3e, 0xfc, 0x7b, 0xc2, 0x7b, 0xfe, 0x7b, 0xce, 0x7f, 0xce, 
        0x7f, 0xfe, 0x6e, 0x3e, 0x57, 0xfe, 0x50, 0x3e, 0x3f, 0xdc, 0x7c, 0x38, 0x7d, 0xf0, 0x3c, 0x00
    };

    static const sprite_t status_icns = { .width = 16, .height = 16, .data = status_icns_data, .mask = nullptr };

    static const uint8_t clock_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x0f, 0xf8, 0x18, 0x0c, 0x37, 0x76, 0x6f, 0xfb, 0x5f, 0x7d, 0x5f, 0x7d, 0x5f, 0x7d, 
        0x4f, 0x09, 0x5f, 0xfd, 0x5f, 0xfd, 0x5f, 0xfd, 0x6f, 0xfb, 0x37, 0x76, 0x18, 0x0c, 0x0f, 0xf8
    };

    static const sprite_t clock_icns = { .width = 16, .height = 16, .data = clock_icns_data, .mask = nullptr };

    static const uint8_t wrench_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x03, 0xe0, 0x07, 0xc0, 0x0f, 0x80, 0x0f, 0x00, 0x0f, 0x00, 0x0f, 0x01, 0x0f, 0x83, 
        0x0f, 0xc7, 0x1f, 0xff, 0x3f, 0xff, 0x6f, 0xfe, 0x5f, 0xfc, 0x3b, 0x00, 0x76, 0x00, 0x6c, 0x00
    };

    static const sprite_t wrench_icns = { .width = 16, .height = 16, .data = wrench_icns_data, .mask = nullptr };

    static ListView * settings_menu = new ListView();
    settings_menu->add_view(new MenuActionItemView("Clock", [this](){ push_submenu(clock_menu); }, &clock_icns));
    settings_menu->add_view(new MenuActionItemView("Status", [this](){ push_submenu(system_info); scroll_guidance->right = false; }, &status_icns));

    main_menu = new ListView();
    main_menu->add_view(new MenuActionItemView("Timer", [this](){ }, &hourglass_icns));
    main_menu->add_view(new MenuActionItemView("Alarm", [this](){ }, &alarm_icns));
    main_menu->add_view(new MenuActionItemView("Settings", [this](){ push_submenu(settings_menu); }, &wrench_icns));
    main_menu->add_view(new MenuActionItemView("Done", []() { change_state(STATE_IDLE); }, &good_icns));

    current_renderable = main_menu;
}   

void AppShimMenu::prepare() {
    scroll_guidance->prepare();
    back_stack.empty();
    current_renderable = main_menu;
    main_menu->switch_to(0, TRANSITION_NONE);
    current_renderable->prepare();
}

void AppShimMenu::render(FantaManipulator*fb) {
    fb->clear();
    current_renderable->render(fb);
    scroll_guidance->render(fb);
}

void AppShimMenu::step() {
    current_renderable->step();
    scroll_guidance->left = !back_stack.empty();
    if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) pop_renderable();
    scroll_guidance->active = hid_test_key_any();
}

void AppShimMenu::push_renderable(Renderable *next) {
    back_stack.push(current_renderable);
    current_renderable->cleanup();
    current_renderable = next;
    current_renderable->prepare();
    scroll_guidance->right = false;
    scroll_guidance->top = false;
    scroll_guidance->bottom = false;
}

void AppShimMenu::push_submenu(ListView *submenu) {
    push_renderable(submenu);
    scroll_guidance->right = true;
    scroll_guidance->top = true;
    scroll_guidance->bottom = true;
}

void AppShimMenu::pop_renderable() {
    if(back_stack.size() == 0) return;
    current_renderable->cleanup();
    current_renderable = back_stack.top();
    back_stack.pop();
    current_renderable->prepare();
}