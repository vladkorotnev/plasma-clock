#include <views/common/compositor.h>

ViewCompositor::ViewCompositor() {
    views = std::list<Renderable*>();
}

void ViewCompositor::add_layer(Renderable * r) {
    views.push_back(r);
}

void ViewCompositor::prepare() {
    for (const auto& v : views) { v->prepare(); }
}

void ViewCompositor::render(FantaManipulator *fb) {
    for (const auto& v : views) { v->render(fb); }
}

void ViewCompositor::step() {
    for (const auto& v : views) { v->step(); }
}

void ViewCompositor::cleanup() {
    for (const auto& v : views) { v->cleanup(); }
}