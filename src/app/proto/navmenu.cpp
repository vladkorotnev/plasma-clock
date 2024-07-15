#include <app/proto/navmenu.h>

ProtoShimNavMenu::ProtoShimNavMenu():
    ProtoShimNavigationStack(new ListView()) {
    main_menu = (ListView*) _root;
}

void ProtoShimNavMenu::prepare() {
    main_menu->switch_to(0, TRANSITION_NONE);
    ProtoShimNavigationStack::prepare();
}