#include <app/proto/navigation_stack.h>
#include <state.h>
#include <input/keys.h>

ProtoShimNavigationStack::ProtoShimNavigationStack(Renderable* root) {
    transition_coordinator = new TransitionAnimationCoordinator();

    scroll_guidance = new TouchArrowOverlay();
    scroll_guidance->top = true;
    scroll_guidance->bottom = true;
    scroll_guidance->right = true;

    _root = root;
    _current_renderable = root;
}


void ProtoShimNavigationStack::push_renderable(Renderable *next, transition_type_t transition) {
    back_stack.push(_current_renderable);
    _current_renderable->cleanup();
    next->prepare();
    set_active_renderable(next, transition);
    scroll_guidance->right = false;
    scroll_guidance->top = false;
    scroll_guidance->bottom = false;
}

void ProtoShimNavigationStack::push_submenu(ListView *submenu) {
    push_renderable(submenu);
    scroll_guidance->right = true;
    scroll_guidance->top = true;
    scroll_guidance->bottom = true;
}

void ProtoShimNavigationStack::pop_renderable(transition_type_t transition) {
    if(back_stack.size() == 0) {
        return;
    }

    _current_renderable->cleanup();
    Renderable* next = back_stack.top();
    back_stack.pop();
    next->prepare();
    set_active_renderable(next, transition);
}

inline Renderable* ProtoShimNavigationStack::current_renderable() {
    if(!transition_coordinator->is_completed()) {
        return transition_coordinator;
    } else {
        return _current_renderable;
    }
}

void ProtoShimNavigationStack::set_active_renderable(Renderable *next, transition_type_t t) {
    if(t != TRANSITION_NONE) {
        transition_coordinator->set_transition(transition_type_to_transition(t));
        transition_coordinator->set_views(_current_renderable, next);
    }

    _current_renderable = next;
}

void ProtoShimNavigationStack::render(FantaManipulator*fb) {
    fb->clear();
    transition_coordinator->update_width(fb->get_width());
    current_renderable()->render(fb);
    scroll_guidance->render(fb);
}

void ProtoShimNavigationStack::step() {
    current_renderable()->step();
    scroll_guidance->left = !back_stack.empty();
    if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) pop_renderable();
    scroll_guidance->active = hid_test_key_any();
}

void ProtoShimNavigationStack::prepare() {
    scroll_guidance->prepare();
    back_stack.empty();
    _current_renderable = _root;
    _current_renderable->prepare();
}