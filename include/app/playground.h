#pragma once
#include <views/framework.h>
#include <device_config.h>

#if HAS(PLAYGROUND)

#include <graphics/font.h>
#include <state.h>

class AppShimPlayground: public Composite {
public:
    AppShimPlayground() {
        scroller = new StringScroll(find_font(FONT_STYLE_UI_TEXT));
        scroller->set_y_position(6);
        scroller->render_mode = slides[slide].first;
        scroller->set_string(slides[slide].second);
        scroller->scroll_only_if_not_fit = false;
        add_composable(scroller);
    }

    void render(FantaManipulator*fb) {
        fb->clear();
        int gridsize = 4;
        bool gridcol = true;
        for(int y = 0; y < fb->get_height(); y += gridsize) {
            gridcol = ((y / 4) % 2) == 0;
            for(int x = 0; x < fb->get_width(); x += gridsize) {
                fb->rect(x, y, x + gridsize, y + gridsize, true, gridcol);
                gridcol ^= 1;
            }
        }
        Composite::render(fb);
    }

    void step() {
        if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
            pop_state(STATE_PLAYGROUND, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        } else if(hid_test_key_any(KEY_RIGHT | KEY_UP | KEY_DOWN) == KEYSTATE_HIT) {
            slide++;
            if(slide >= slides.size()) slide = 0;
            scroller->render_mode = slides[slide].first;
            scroller->set_string(slides[slide].second);
        }

        Composite::step();
    }

private:
    StringScroll * scroller;
    int slide = 0;
    std::vector<std::pair<text_attributes_t, const char *>> slides = {
        {TEXT_NORMAL, "Normal"},
        {TEXT_NO_BACKGROUND, "No Bg"},
        {TEXT_INVERTED, "Invert"},
        {TEXT_OUTLINED, "Outline"},
        {TEXT_OUTLINED | OUTLINE_INVERTED, "OutInv"},
        {TEXT_NO_BACKGROUND | TEXT_INVERTED, "No Bg Inv"},
        {TEXT_NO_BACKGROUND | TEXT_OUTLINED, "No Bg Outl"},
        {TEXT_NO_BACKGROUND | TEXT_OUTLINED | OUTLINE_INVERTED, "No Bg InvOutl"},
        {TEXT_NO_BACKGROUND | TEXT_INVERTED | TEXT_OUTLINED, "NoBg Inv Out"},
        {TEXT_NO_BACKGROUND | TEXT_INVERTED | TEXT_OUTLINED | OUTLINE_INVERTED, "NoBg Inv InvOut"},
    };
};

#endif