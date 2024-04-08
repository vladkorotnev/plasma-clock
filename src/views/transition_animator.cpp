#include <views/transition_animator.h>

TransitionAnimator::TransitionAnimator(Renderable * sourceView, Renderable * targetView, int duration) { 
    viewA = sourceView;
    viewB = targetView;
    dur = duration;

    backingBufferA = (fanta_buffer_t) malloc(PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE);
    backingBufferB = (fanta_buffer_t) malloc(PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE);
    backingA = new FantaManipulator(backingBufferA, PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE, PlasmaDisplayFramebuffer::width, PlasmaDisplayFramebuffer::height, semaA, &dirtyA);
    backingB = new FantaManipulator(backingBufferB, PlasmaDisplayFramebuffer::PDFB_BUFFER_SIZE, PlasmaDisplayFramebuffer::width, PlasmaDisplayFramebuffer::height, semaB, &dirtyB);
    backingA->clear();
    backingB->clear();
}

TransitionAnimator::~TransitionAnimator() {
    delete backingA;
    delete backingB;
}

void TransitionAnimator::step() {
    viewA->step();
    viewB->step();
}

void TransitionAnimator::prepare() {
    viewA->prepare();
    viewB->prepare();
}

void TransitionAnimator::render_backing_views()  {
    backingA->clear();
    backingB->clear();
    viewA->render(backingA);
    viewB->render(backingB);
}