#pragma once
#include <vector>
#include <views/framework.h>
#include <views/overlays/touch_arrows_ovl.h>
#include <views/overlays/scrollbar.h>

class ListView: public ViewMultiplexor {
public:
    static const view_id_t VIEW_ID_AUTOMATIC = 0xFFFF;
    bool selectable = true;
    ListView();
    ~ListView();

    void add_view(Renderable*, view_id_t = VIEW_ID_AUTOMATIC);
    void switch_to(view_id_t, transition_type_t);
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
    ScrollbarOverlay * scrollbar;
    std::vector<view_id_t> view_ids;

private:
    bool select_flag;
    int current_idx;
};