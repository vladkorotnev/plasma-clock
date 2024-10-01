#pragma once
#include "view.h"
#include <list>

/// @brief Combines multiple Renderables in the Z axis
class ViewCompositor: public Renderable {
public:
    ViewCompositor();
    
    void add_layer(Renderable*);
    
    void prepare();
    void render_plane(FantaManipulator*, RenderPlane);
    void step();
    void cleanup();
private:
    std::list<Renderable*> views;
};

class ScreenCompositor: public DisplayTimeable, public ViewCompositor {
public:
    ScreenCompositor(Screen* root): ViewCompositor() {
        _root = root;
        if(_root != nullptr) add_layer(_root);
    }

    int desired_display_time() {
        if(_root == nullptr) return DISP_TIME_NO_OVERRIDE;
        return _root->desired_display_time();
    }

    void prepare() { ViewCompositor::prepare(); }
    void render_plane(FantaManipulator *fb, RenderPlane p) { ViewCompositor::render_plane(fb, p); }
    void step() { ViewCompositor::step(); }
    void cleanup() { ViewCompositor::cleanup(); }

private:
    Screen * _root;
};