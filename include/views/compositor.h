#pragma once
#include "view.h"
#include <list>

class ViewCompositor: public Renderable {
public:
    ViewCompositor();
    
    void add_layer(Renderable*);
    
    void prepare();
    void render(FantaManipulator*);
    void step();
    void cleanup();
private:
    std::list<Renderable*> views;
};