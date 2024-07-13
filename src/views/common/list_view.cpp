#include <esp32-hal-log.h>
#include <views/common/list_view.h>
#include <input/keys.h>
#include <algorithm>

ListView::ListView() : ViewMultiplexor() {
    scroll_guidance = new TouchArrowOverlay();
    scroll_guidance->top = true;
    scroll_guidance->bottom = true;
#if HAS(TOUCH_PLANE)
    scroll_guidance->active = true;
#endif
    scrollbar = new ScrollbarOverlay();
    view_ids = {};
    current_idx = -1;
    select_flag = false;
}

ListView::~ListView() {
    delete scroll_guidance;
    delete scrollbar;
}

void ListView::prepare() {
    scroll_guidance->prepare();
    ViewMultiplexor::prepare();
}

void ListView::render(FantaManipulator * fb) {
    ViewMultiplexor::render(fb);
    scroll_guidance->render(fb);
    scrollbar->render(fb);
}

void ListView::step() {
    ViewMultiplexor::step();
    scroll_guidance->right = selectable;

    if(hid_test_key_state_repetition(KEY_DOWN)) scroll_down();
    else if(hid_test_key_state_repetition(KEY_UP)) scroll_up();
    else if(selectable && hid_test_key_state(KEY_RIGHT)) {
        select_flag = true;
    }
}

void ListView::scroll_up() {
    if(current_idx == 0) {
        current_idx = view_ids.size();
    }
    if(current_idx == -1) return;
    current_idx--;

    switch_to(view_ids[current_idx], TRANSITION_SLIDE_VERTICAL_DOWN);
}

void ListView::scroll_down() {
    if(current_idx == -1 || view_ids.size() == 0) return;
    
    if(current_idx == view_ids.size() - 1) {
        current_idx = 0;
    } else {
        current_idx ++;
    }

    switch_to(view_ids[current_idx], TRANSITION_SLIDE_VERTICAL_UP);
}

void ListView::cleanup() {
    scroll_guidance->cleanup();
    scrollbar->cleanup();
    ViewMultiplexor::cleanup();
}

void ListView::add_view(Renderable *view, view_id_t id) {
    ViewMultiplexor::add_view(view, id);
    view_ids.push_back(id);
    if(current_idx == -1) {
        current_idx = 0;
    }
}

void ListView::remove_view(view_id_t id) {
    ViewMultiplexor::remove_view(id);
    if(view_ids[current_idx] == id) {
        current_idx = -1;
    }
    view_ids.erase(std::remove(view_ids.begin(), view_ids.end(), 8), view_ids.end());
}

bool ListView::did_select() {
    bool tmp = select_flag;
    select_flag = false;
    return tmp;
}

view_id_t ListView::get_selection() {
    if(current_idx == -1) return -1;
    return view_ids[current_idx];
}

