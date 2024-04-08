#pragma once
#include "view.h"
#include "views/transitions.h"
#include "transition_animator.h"
#include <map>

typedef uint16_t view_id_t;

class ViewMultiplexor: public Renderable {
public: 
    ViewMultiplexor();
    ~ViewMultiplexor();

    void add_view(Renderable*, view_id_t);
    void switch_to(view_id_t);
    void remove_view(view_id_t);

    void prepare();
    void render(FantaManipulator*);
    void step();
    void cleanup();
private:
    std::map<view_id_t, Renderable*> views;
    Renderable * active_view;
    void set_active_view(Renderable*, transition_type_t);
    TransitionAnimationCoordinator * transition_coordinator;
    Renderable* current_renderable();
};