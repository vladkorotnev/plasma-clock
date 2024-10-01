#pragma once
#include <views/common/view.h>
#include <graphics/framebuffer.h>

class TransitionAnimationCoordinator;

class Transition {
public:
    Transition() {}

    /// @brief Render the current frame of the transition into the FantaManipulator.
    /// @param fb Target (usually a screen)
    /// @param  coord Transition coordinator
    /// @return True if transition has finished, false otherwise.
    virtual bool render(FantaManipulator * fb, TransitionAnimationCoordinator * coord) { return true; }
    /// @brief Prepare the object to render the transition again
    virtual void rewind() {}
};

class TransitionAnimationCoordinator: public Renderable {
public:
    explicit TransitionAnimationCoordinator();
    ~TransitionAnimationCoordinator();

    void set_transition(Transition*);
    void set_views(Renderable *A, Renderable *B);
    void rewind();
    bool is_completed();

    void prepare();
    void step();
    void render_plane(FantaManipulator *, RenderPlane) override;

    fanta_buffer_t backingBufferA;
    fanta_buffer_t backingBufferB;
    FantaManipulator * backingA;
    FantaManipulator * backingB;
    Renderable * viewA;
    Renderable * viewB;
private:
    SemaphoreHandle_t semaA;
    SemaphoreHandle_t semaB;
    bool dirtyA;
    bool dirtyB;
    bool completed = true;
    Transition *transition;

    void render_backing_views(RenderPlane);
};