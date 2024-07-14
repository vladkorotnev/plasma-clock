#pragma once
#include <stack>
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sensor/sensor.h>

class AppShimMenu: public Renderable {
public:
    AppShimMenu(Beeper*);

    void prepare();
    void render(FantaManipulator*fb);
    void step();

    void push_submenu(ListView*);
    void push_renderable(Renderable*);
    void pop_renderable();

private:
    TransitionAnimationCoordinator * transition_coordinator;
    TouchArrowOverlay * scroll_guidance;
    Renderable* _current_renderable;
    void set_active_renderable(Renderable*, transition_type_t);
    Renderable* current_renderable();
    ListView* main_menu;
    std::stack<Renderable*> back_stack;
    Beeper * beeper;
    TickType_t last_touch_time;
};