#pragma once
#include <views/framework.h>

class ScrollbarOverlay: public Renderable {
public:
    int total = 0;
    int selected = 0;
    ScrollbarOverlay() {}

    void render(FantaManipulator* fb) {
        if(total == 0) return;

        int quacklet_height = std::max(1, fb->get_height() / total);
        for(int i = 0; i < fb->get_height(); i++) {
            fb->plot_pixel(fb->get_width() - 1, i, (i >= selected * quacklet_height && i < (selected + 1)*quacklet_height));
        }
    }
};