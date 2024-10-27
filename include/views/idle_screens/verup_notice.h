#pragma once
#include <views/framework.h>

class VerUpNoticeView: public Screen {
public:
    VerUpNoticeView() {}
    void prepare() override;
    void step() override;
    int desired_display_time() override;
private:
    StringScroll * title = nullptr;
    StringScroll * message = nullptr;
    SpriteView * icon = nullptr;
    void create_components_if_needed();
};