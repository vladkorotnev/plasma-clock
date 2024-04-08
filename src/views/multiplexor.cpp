#include <views/multiplexor.h>

ViewMultiplexor::ViewMultiplexor() {
    views = std::map<view_id_t, Renderable*>();
    active_view = nullptr;
}

ViewMultiplexor::~ViewMultiplexor() {
    active_view = nullptr;
    for (const auto& kv : views) {
        kv.second->cleanup();
        delete kv.second;
    }
}

void ViewMultiplexor::set_active_view(Renderable *next) {
    if(active_view != nullptr) {
        active_view->cleanup();
    }

    active_view = next;
    
    if(active_view != nullptr) {
        active_view->prepare();
    }
}

void ViewMultiplexor::add_view(Renderable *view, view_id_t id) {
    if(views.count(id) > 0) return;

    views[id] = view;
    if(active_view == nullptr) {
        set_active_view(view);
    }
}

void ViewMultiplexor::switch_to(view_id_t id) {
    Renderable *view = views[id];
    if(view == nullptr) return;

    set_active_view(view);
}

void ViewMultiplexor::remove_view(view_id_t id) {
    Renderable *view = views[id];
    if(view == nullptr) return;

    if(active_view == view) {
        set_active_view(nullptr);
    }

    views.erase(id);
}

void ViewMultiplexor::prepare() {
    if(active_view != nullptr) {
        active_view->prepare();
    }
}

void ViewMultiplexor::render(FantaManipulator * fb) {
    if(active_view != nullptr) {
        active_view->render(fb);
    }
}

void ViewMultiplexor::step() {
    if(active_view != nullptr) {
        active_view->step();
    }
}

void ViewMultiplexor::cleanup() {
    if(active_view != nullptr) {
        active_view->cleanup();
    }
}