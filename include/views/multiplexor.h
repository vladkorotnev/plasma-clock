#pragma once
#include "view.h"
#include "views/transitions.h"
#include "transition_animator.h"
#include <map>

typedef uint16_t view_id_t;

/// @brief Switches across multiple Screens as desired
class ViewMultiplexor: public Renderable {
public: 
    ViewMultiplexor();
    ~ViewMultiplexor();

    void add_view(Screen*, view_id_t);
    void switch_to(view_id_t, transition_type_t);
    void remove_view(view_id_t);
    Screen * get_view(view_id_t);

    void prepare();
    void render(FantaManipulator*);
    void step();
    void cleanup();
private:
    std::map<view_id_t, Screen*> views;
    Screen * active_screen;
    void set_active_screen(Screen*, transition_type_t);
    TransitionAnimationCoordinator * transition_coordinator;
    Renderable* current_renderable();
};