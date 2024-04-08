#include <views/transition_animator.h>

TransitionAnimationCoordinator::TransitionAnimationCoordinator() { 
    backingBufferA = (fanta_buffer_t) malloc(PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE);
    backingBufferB = (fanta_buffer_t) malloc(PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE);
    backingA = new FantaManipulator(backingBufferA, PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE, PlasmaDisplayFramebuffer::width, PlasmaDisplayFramebuffer::height, semaA, &dirtyA);
    backingB = new FantaManipulator(backingBufferB, PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE, PlasmaDisplayFramebuffer::width, PlasmaDisplayFramebuffer::height, semaB, &dirtyB);
    backingA->clear();
    backingB->clear();
    transition = nullptr;
}

TransitionAnimationCoordinator::~TransitionAnimationCoordinator() {
    delete backingA;
    delete backingB;
}

void TransitionAnimationCoordinator::step() {
    viewA->step();
    viewB->step();
}

void TransitionAnimationCoordinator::prepare() {
    viewA->prepare();
    viewB->prepare();
}

void TransitionAnimationCoordinator::render_backing_views()  {
    backingA->clear();
    backingB->clear();
    viewA->render(backingA);
    viewB->render(backingB);
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