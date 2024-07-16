#pragma once
#include <stack>
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sensor/sensor.h>

class ProtoShimNavigationStack: public Renderable {
public:
    ProtoShimNavigationStack(Renderable* root);

    void prepare();
    void render(FantaManipulator*fb);
    void step();

    void push_submenu(ListView*);
    void push_renderable(Renderable*, transition_type_t = TRANSITION_SLIDE_HORIZONTAL_LEFT);
    virtual void pop_renderable(transition_type_t = TRANSITION_SLIDE_HORIZONTAL_RIGHT);

protected:
    TransitionAnimationCoordinator * transition_coordinator;
    TouchArrowOverlay * scroll_guidance;
    Renderable* _root;
    Renderable* _current_renderable;
    void set_active_renderable(Renderable*, transition_type_t);
    Renderable* current_renderable();
    std::stack<Renderable*> back_stack;
};