#include "views/weather/current_weather.h"
#include <service/owm/weather_icons.h>
#include <graphics/font.h>
#include <esp32-hal-log.h>
#include <service/localize.h>

static char LOG_TAG[] = "WTHVW";

CurrentWeatherView::CurrentWeatherView() {
    icon = { 0 };
    weather = { 0 };
    weather.conditions = weather_condition_code::UNKNOWN_CONDITIONS;

    animation = new AniSpriteView(nullptr);
    animation->x_offset = 0;
    animation->width = 16;

    big_font = find_font(FONT_STYLE_TALL_TEXT);
    small_font = find_font(FONT_STYLE_UI_TEXT);

    bottom_line = new StringScroll(small_font);
    bottom_line->set_y_position(8);
    bottom_line->x_offset = 17;

    top_line = new StringScroll(small_font);
    top_line->set_y_position(0);
    top_line->stopped = true;
    top_line->start_at_visible = true;
    top_line->x_offset = 17;

    add_composable(top_line);
    add_composable(bottom_line);
    add_composable(animation);
    ESP_LOGV(LOG_TAG, "Init");
}

CurrentWeatherView::~CurrentWeatherView() {
    delete bottom_line;
    delete top_line;
    delete animation;
}

void CurrentWeatherView::prepare() {
    if(weather_get_current(&weather)) {
        ESP_LOGV(LOG_TAG, "Initial fetch succeeded");
        prepare_for_new_weather();
    }

    Screen::prepare();
}

void CurrentWeatherView::prepare_for_new_weather() {
    icon = sprite_from_conditions(weather.conditions);
    if(icon->data != nullptr) {
        animation->set_sprite(icon);
    }

    snprintf(top_text, sizeof(top_text), "%.01f°%c %i%%", convert_temperature(KELVIN, weather.temperature_kelvin), preferred_temperature_unit(), weather.humidity_percent);
    snprintf(bottom_text, sizeof(bottom_text), localized_string("WEATHER_FMT"), weather.description, convert_temperature(KELVIN, weather.feels_like_kelvin), preferred_temperature_unit(), weather.windspeed_mps, weather.pressure_hpa);
    top_line->set_string(top_text);
    bottom_line->set_string(bottom_text);
}

void CurrentWeatherView::render(FantaManipulator* fb) {
    if(weather.conditions == UNKNOWN_CONDITIONS) {
        fb->put_string(big_font, localized_string("Loading..."), 4, 0);
    } else {   
        Screen::render(fb);
    }
}

int CurrentWeatherView::desired_display_time() {
    int estimated_ms = (bottom_line->estimated_frame_count() * 1000 / 58);
    return estimated_ms > 0 ? estimated_ms : DISP_TIME_NO_OVERRIDE;
}