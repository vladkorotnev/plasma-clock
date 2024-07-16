#include <esp32-hal-log.h>
#include <views/common/list_view.h>
#include <input/keys.h>
#include <algorithm>

ListView::ListView() : ViewMultiplexor() {
    scrollbar = new ScrollbarOverlay();
    view_ids = {};
    current_idx = -1;
    select_flag = false;
}

ListView::~ListView() {
    delete scrollbar;
}

void ListView::prepare() {
    ViewMultiplexor::prepare();
}

void ListView::render(FantaManipulator * fb) {
    ViewMultiplexor::render(fb);
    scrollbar->render(fb);
}

void ListView::step() {
    ViewMultiplexor::step();

    if(hid_test_key_state_repetition(KEY_DOWN)) scroll_down();
    else if(hid_test_key_state_repetition(KEY_UP)) scroll_up();
}

void ListView::scroll_up() {
    if(current_idx == 0) {
        current_idx = view_ids.size();
    }
    if(current_idx == -1) return;
    current_idx--;
    scrollbar->selected = current_idx;

    switch_to(view_ids[current_idx], TRANSITION_SLIDE_VERTICAL_DOWN);
}

void ListView::scroll_down() {
    if(current_idx == -1 || view_ids.size() == 0) return;
    
    if(current_idx == view_ids.size() - 1) {
        current_idx = 0;
    } else {
        current_idx ++;
    }
    scrollbar->selected = current_idx;

    switch_to(view_ids[current_idx], TRANSITION_SLIDE_VERTICAL_UP);
}

void ListView::switch_to(view_id_t id, transition_type_t transition) {
    ViewMultiplexor::switch_to(id, transition);
    auto idx = std::find(view_ids.begin(), view_ids.end(), id);
    if(idx != view_ids.end()) {
        current_idx = distance(view_ids.begin(), idx);
        scrollbar->selected = current_idx;
    }
}

void ListView::cleanup() {
    scrollbar->cleanup();
    ViewMultiplexor::cleanup();
}

void ListView::add_view(Renderable *view, view_id_t id) {
    if(id == VIEW_ID_AUTOMATIC) id = view_ids.size();
    
    ViewMultiplexor::add_view(view, id);
    view_ids.push_back(id);
    scrollbar->total = view_ids.size();
    if(current_idx == -1) {
        current_idx = 0;
        scrollbar->selected = 0;
    }
}

void ListView::remove_view(view_id_t id) {
    ViewMultiplexor::remove_view(id);
    if(view_ids[current_idx] == id) {
        current_idx = -1;
        scrollbar->selected = -1;
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

