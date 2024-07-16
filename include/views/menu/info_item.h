#pragma once
#include <functional>
#include <views/framework.h>
#include <fonts.h>

class MenuInfoItemView: public Composite {
public:
    MenuInfoItemView(const char * title, const char * subtitle);
    ~MenuInfoItemView();
    
private:
    StringScroll * top_label;
    StringScroll * bottom_label;
};
