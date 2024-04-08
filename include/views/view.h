#pragma once
#include <plasma/fanta_manipulator.h>

class Renderable {
public:
    virtual void prepare() { }
    virtual void render(FantaManipulator*) { }
    virtual void step() { }
    virtual void cleanup() { }
};