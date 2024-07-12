#include <views/common/multiplexor.h>
#include <esp32-hal-log.h>

ViewMultiplexor::ViewMultiplexor() {
    views = std::map<view_id_t, Renderable*>();
    active_screen = nullptr;
    transition_coordinator = new TransitionAnimationCoordinator();
}

ViewMultiplexor::~ViewMultiplexor() {
    delete transition_coordinator;
    active_screen = nullptr;
    for (const auto& kv : views) {
        kv.second->cleanup();
        delete kv.second;
    }
}

inline Renderable* ViewMultiplexor::current_renderable() {
    if(!transition_coordinator->is_completed()) {
        return transition_coordinator;
    } else {
        return active_screen;
    }
}

void ViewMultiplexor::set_active_screen(Renderable *next, transition_type_t t) {
    Renderable *r = current_renderable();
    if(r != nullptr) r->cleanup();

    if(t != TRANSITION_NONE) {
        transition_coordinator->set_transition(transition_type_to_transition(t));
        transition_coordinator->set_views(active_screen, next);
    }

    active_screen = next;
    
    r = current_renderable();
    r->prepare();
}

Renderable * ViewMultiplexor::get_view(view_id_t id) {
    return views[id];
}

void ViewMultiplexor::add_view(Renderable *view, view_id_t id) {
    if(views.count(id) > 0) return;
    views[id] = view;
    if(active_screen == nullptr) {
        set_active_screen(view, TRANSITION_NONE);
    }
}

void ViewMultiplexor::switch_to(view_id_t id, transition_type_t transition) {
    Renderable *view = views[id];
    if(view == nullptr || view == active_screen) return;

    if(active_screen == nullptr) {
        set_active_screen(view, TRANSITION_NONE);
    } else {
        set_active_screen(view, transition);
    }
}

void ViewMultiplexor::remove_view(view_id_t id) {
    Renderable *view = views[id];
    if(view == nullptr) return;

    if(active_screen == view) {
        set_active_screen(nullptr, TRANSITION_NONE);
    }

    views.erase(id);
}

void ViewMultiplexor::prepare() {
    Renderable *r = current_renderable();
    if(r != nullptr) r->prepare();
}

void ViewMultiplexor::render(FantaManipulator * fb) {
    Renderable *r = current_renderable();
    if(r != nullptr) r->render(fb);
}

void ViewMultiplexor::step() {
    Renderable *r = current_renderable();
    if(r != nullptr) r->step();
}

void ViewMultiplexor::cleanup() {
    Renderable *r = current_renderable();
    if(r != nullptr) r->cleanup();
}