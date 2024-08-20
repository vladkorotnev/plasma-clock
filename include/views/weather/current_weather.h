#pragma once
#include <views/framework.h>
#include <views/common/string_scroll.h>
#include <service/owm/weather.h>

class CurrentWeatherView: public Screen {
public:
    CurrentWeatherView();
    ~CurrentWeatherView();
    void prepare();
    void render(FantaManipulator*);
    int desired_display_time();

private:
    const font_definition_t * big_font;
    const font_definition_t * small_font;
    StringScroll * bottom_line;
    StringScroll * top_line;
    char bottom_text[128];
    char top_text[32];
    current_weather_t weather;
    AniSpriteView * animation;
    const ani_sprite_t * icon;
    void prerender_icon_frame();
    void prepare_for_new_weather();
};