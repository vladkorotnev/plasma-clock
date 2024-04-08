#include <views/multiplexor.h>
#include <views/transitions.h>

ViewMultiplexor::ViewMultiplexor() {
    views = std::map<view_id_t, Renderable*>();
    active_view = nullptr;
    active_transition = nullptr;
}

ViewMultiplexor::~ViewMultiplexor() {
    active_view = nullptr;
    for (const auto& kv : views) {
        kv.second->cleanup();
        delete kv.second;
    }
}

void ViewMultiplexor::set_active_view(Renderable *next, TransitionAnimator * animator) {
    if(active_transition != nullptr) {
        active_transition->cleanup();
        delete active_transition;
        active_transition = nullptr;
    }

    if(active_view != nullptr) {
        active_view->cleanup();
    }

    active_view = next;
    active_transition = animator;
    
    if(active_transition != nullptr) {
        active_transition->prepare();
    } else if(active_view != nullptr) {
        active_view->prepare();
    }
}

void ViewMultiplexor::add_view(Renderable *view, view_id_t id) {
    if(views.count(id) > 0) return;

    views[id] = view;
    if(active_view == nullptr) {
        set_active_view(view, nullptr);
    }
}

void ViewMultiplexor::switch_to(view_id_t id) {
    Renderable *view = views[id];
    if(view == nullptr) return;

    if(active_view == nullptr) {
        set_active_view(view, nullptr);
    } else {
        set_active_view(view, new WipeTransition(active_view, view, 101));
    }
}

void ViewMultiplexor::remove_view(view_id_t id) {
    Renderable *view = views[id];
    if(view == nullptr) return;

    if(active_view == view) {
        set_active_view(nullptr, nullptr);
    }

    views.erase(id);
}

void ViewMultiplexor::prepare() {
    if(active_transition != nullptr) {
        active_transition->prepare();
    } else if(active_view != nullptr) {
        active_view->prepare();
    }
}

void ViewMultiplexor::render(FantaManipulator * fb) {
    if(active_transition != nullptr) {
        active_transition->render(fb);
        if(active_transition->is_complete()) {
            delete active_transition;
            active_transition = nullptr;
        }
    } else if(active_view != nullptr) {
        active_view->render(fb);
    }
}

void ViewMultiplexor::step() {
    if(active_transition != nullptr) {
        active_transition->step();
    } else if(active_view != nullptr) {
        active_view->step();
    }
}

void ViewMultiplexor::cleanup() {
     if(active_transition != nullptr) {
        active_transition->cleanup();
    } else if(active_view != nullptr) {
        active_view->cleanup();
    }
}