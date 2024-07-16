#include <views/transitions/transition_animator.h>

TransitionAnimationCoordinator::TransitionAnimationCoordinator() { 
    backingBufferA = (fanta_buffer_t) malloc(DisplayFramebuffer::BUFFER_SIZE);
    backingBufferB = (fanta_buffer_t) malloc(DisplayFramebuffer::BUFFER_SIZE);
    backingA = new FantaManipulator(backingBufferA, DisplayFramebuffer::BUFFER_SIZE, DisplayFramebuffer::width, DisplayFramebuffer::height, semaA, &dirtyA);
    backingB = new FantaManipulator(backingBufferB, DisplayFramebuffer::BUFFER_SIZE, DisplayFramebuffer::width, DisplayFramebuffer::height, semaB, &dirtyB);
    backingA->clear();
    backingB->clear();
    transition = nullptr;
}

TransitionAnimationCoordinator::~TransitionAnimationCoordinator() {
    delete backingA;
    delete backingB;
}

void TransitionAnimationCoordinator::step() {
    if(viewA != nullptr) viewA->step();
    if(viewB != nullptr) viewB->step();
}

void TransitionAnimationCoordinator::prepare() {
    // viewA is expected to be already prepared from it's previous state
    if(viewB != nullptr) viewB->prepare();
}

void TransitionAnimationCoordinator::render_backing_views()  {
    backingA->clear();
    backingB->clear();
    if(viewA != nullptr) viewA->render(backingA);
    if(viewB != nullptr) viewB->render(backingB);
}

bool TransitionAnimationCoordinator::is_completed() {
    return completed;
}

void TransitionAnimationCoordinator::set_transition(Transition *t) {
    transition = t;
    if(transition != nullptr) {
        transition->rewind();
        completed = false;
    } else {
        completed = true;
    }
}

void TransitionAnimationCoordinator::set_views(Renderable *A, Renderable *B) {
    viewA = A;
    viewB = B;
}

void TransitionAnimationCoordinator::rewind() {
    if(transition == nullptr) { return; }
    transition->rewind();
    completed = false;
}

void TransitionAnimationCoordinator::render(FantaManipulator *fb) {
    if(transition == nullptr) { return; }
    render_backing_views();
    completed = transition->render(fb, this);
}