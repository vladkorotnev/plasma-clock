#pragma once
#include "view.h"
#include <plasma/framebuffer.h>

class TransitionAnimator: public Renderable {
public:
    explicit TransitionAnimator(Renderable * sourceView, Renderable * targetView, int duration);
    ~TransitionAnimator();

    virtual bool is_complete() { return false; }

    void prepare();
    void step();

    Renderable * viewA;
    Renderable * viewB;
    int dur;
protected:
    fanta_buffer_t backingBufferA;
    fanta_buffer_t backingBufferB;
    FantaManipulator * backingA;
    FantaManipulator * backingB;
    SemaphoreHandle_t semaA;
    SemaphoreHandle_t semaB;
    bool dirtyA;
    bool dirtyB;

    void render_backing_views();
};