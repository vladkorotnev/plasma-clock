#pragma once
#include <vector>
#include <views/framework.h>
#include <views/overlays/touch_arrows_ovl.h>
#include <views/overlays/scrollbar.h>

class ListView: public ViewMultiplexor {
public:
    bool selectable = true;
    ListView();
    ~ListView();

    void add_view(Renderable*, view_id_t);
    void remove_view(view_id_t);

    void prepare();
    void render(FantaManipulator *);
    void step();
    void cleanup();

    void scroll_down();
    void scroll_up();
    bool did_select();
    view_id_t get_selection();

protected:
    TouchArrowOverlay * scroll_guidance;
    ScrollbarOverlay * scrollbar;
    std::vector<view_id_t> view_ids;

private:
    bool ignoring_touches;
    bool select_flag;
    int current_idx;
};