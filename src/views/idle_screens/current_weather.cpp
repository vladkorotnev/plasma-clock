#include "views/idle_screens/current_weather.h"
#include <service/owm/weather_icons.h>
#include <fonts.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "WTHVW";

CurrentWeatherView::CurrentWeatherView() {
    weather = { 0 };
    weather.conditions = weather_condition_code::UNKNOWN;
    icon_state = { 0 };
    current_icon_frame = { 0 };
    icon = { 0 };
    big_font = &xnu_font;
    small_font = &keyrus0808_font;
    bottom_line = new StringScroll(small_font);
    bottom_line->set_y_position(8);
    ESP_LOGV(LOG_TAG, "Init");
}

CurrentWeatherView::~CurrentWeatherView() {
    delete bottom_line;
}

void CurrentWeatherView::prepare() {
    if(weather_get_current(&weather)) {
        ESP_LOGV(LOG_TAG, "Initial fetch succeeded");
        prepare_for_new_weather();
    }

    bottom_line->prepare();
}

void CurrentWeatherView::step() {
    current_weather_t w;
    if(weather_get_current(&w) && w.last_updated != weather.last_updated) {
        weather = w;
        prepare_for_new_weather();
    } else {
        prerender_icon_frame();
    }

    bottom_line->step();
}

void CurrentWeatherView::cleanup() {
    bottom_line->cleanup();
}

void CurrentWeatherView::prepare_for_new_weather() {
    icon = sprite_from_conditions(weather.conditions);
    if(icon.data != nullptr) {
        icon_state = ani_sprite_prepare(&icon);
        prerender_icon_frame();
    }

    snprintf(bottom_text, sizeof(bottom_text), "%s. Feels like %.01f\370C. Wind %.01f m/s. Pressure %i hPa.", weather.description, kelvin_to(weather.feels_like_kelvin, CELSIUS), weather.windspeed_mps, weather.pressure_hpa);
    bottom_line->set_string(bottom_text);
}

void CurrentWeatherView::prerender_icon_frame() {
    if(icon.data == nullptr || icon_state.ani_sprite == nullptr) return;
    current_icon_frame = ani_sprite_frame(&icon_state);
}

ani_sprite_t CurrentWeatherView::sprite_from_conditions(weather_condition_t conditions) {
    switch(conditions) {
        case THUNDERSTORM: return thunderstorm_icon;
        case DRIZZLE: return drizzle_icon;
        case RAIN: return rain_icon;
        case SNOW: return snow_icon;
        case MIST: return mist_icon;
        case CLEAR: return sun_icon;
        case CLOUDS: return clouds_icon;
        case SCATTERED_CLOUDS: return clouds_icon;
        case BROKEN_CLOUDS: return broken_clouds_icon;
        case OVERCAST: return overcast_icon;

        case UNKNOWN:
        default:
            return ani_sprite_t { 0 };
    }
}

void CurrentWeatherView::render(FantaManipulator* fb) {
    if(weather.conditions == UNKNOWN) {
        fb->put_string(big_font, "Loading...", 4, 0);
    } else {
        if(current_icon_frame.data != nullptr) {
            fb->put_sprite(&current_icon_frame, 0, 0);
        }
        
        FantaManipulator * text_window = fb->slice(17, fb->get_width() - 17);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.01f\370C %i%%", kelvin_to(weather.temperature_kelvin, CELSIUS), weather.humidity_percent);
        text_window->put_string(small_font, buf, 0, 0);

        bottom_line->render(text_window);

        delete text_window;
    }
}

int CurrentWeatherView::desired_display_time() {
    return std::max((bottom_line->estimated_frame_count() * 1000 / 58) + 2000, 1); // workaround when the bottom string is null 
}