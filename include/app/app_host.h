#include <views/framework.h>

class AppHost: public Composite {
public:
    AppHost();
    void render(FantaManipulator*) override;
    void step() override;
    void add_view(Renderable*, view_id_t);
    void switch_to(view_id_t, transition_type_t);

protected:
    bool is_in_critical_state();
    bool state_wants_full_screen(view_id_t);
    ViewMultiplexor * main_screen = nullptr;
    ViewMultiplexor * split_screen = nullptr;
    bool split_active = false;
    bool main_idle = true;
};