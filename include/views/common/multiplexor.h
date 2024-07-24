#pragma once
#include <views/common/view.h>
#include <views/transitions/transitions.h>
#include <views/transitions/transition_animator.h>
#include <map>

typedef uint16_t view_id_t;

/// @brief Switches across multiple Renderables as desired
class ViewMultiplexor: public Composable {
public: 
    ViewMultiplexor();
    ~ViewMultiplexor();

    void add_view(Renderable*, view_id_t);
    void switch_to(view_id_t, transition_type_t);
    void remove_view(view_id_t);
    Renderable * get_view(view_id_t);

    void prepare();
    void render(FantaManipulator*);
    void step();
    void cleanup();
protected:
    std::map<view_id_t, Renderable*> views;
    Renderable * active_screen;
    void set_active_screen(Renderable*, transition_type_t);
    TransitionAnimationCoordinator * transition_coordinator;
    Renderable* current_renderable();
};