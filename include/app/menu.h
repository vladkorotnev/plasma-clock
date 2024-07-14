#pragma once
#include <stack>
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sensor/sensor.h>

class AppShimMenu: public Renderable {
public:
    AppShimMenu();

    void prepare();
    void render(FantaManipulator*fb);
    void step();

    void push_submenu(ListView*);
    void push_renderable(Renderable*);
    void pop_renderable();

private:
    TouchArrowOverlay * scroll_guidance;
    Renderable* current_renderable;
    ListView* main_menu;
    std::stack<Renderable*> back_stack;
};