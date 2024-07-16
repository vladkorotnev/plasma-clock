#pragma once
#include "navigation_stack.h"

class ProtoShimNavMenu: public ProtoShimNavigationStack {
public:
    ProtoShimNavMenu();
    void prepare();
protected:
    ListView* main_menu;
};
