#pragma once
#include <functional>
#include <views/framework.h>
#include <graphics/font.h>

class MenuInfoItemView: public Composite {
public:
    MenuInfoItemView(const char * title, const char * subtitle);
    ~MenuInfoItemView();
    
protected:
    StringScroll * top_label;
    StringScroll * bottom_label;
};
