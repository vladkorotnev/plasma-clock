#pragma once
#include <views/framework.h>
#include <views/string_scroll.h>
#include <service/owm/weather.h>

class CurrentWeatherView: public Screen {
public:
    CurrentWeatherView();
    ~CurrentWeatherView();
    void prepare();
    void step();
    void cleanup();
    void render(FantaManipulator*);

private:
    const font_definition_t * big_font;
    const font_definition_t * small_font;
    StringScroll * bottom_line;
    char bottom_text[128];
    current_weather_t weather;
    ani_sprite icon;
    ani_sprite_state_t icon_state;
    sprite_t current_icon_frame;
    ani_sprite_t sprite_from_conditions(weather_condition_t);
    void prerender_icon_frame();
    void prepare_for_new_weather();
};