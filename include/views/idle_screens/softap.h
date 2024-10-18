#pragma once
#include <device_config.h>
#include <views/framework.h>

class SoftApInfoView: public Screen {
public:
    SoftApInfoView() {}
    void prepare() override;
    int desired_display_time() override;
private:
    StringScroll * network_name = nullptr;
    StringScroll * ip_address = nullptr;
    SpriteView * icon = nullptr;
    void create_components_if_needed();
};