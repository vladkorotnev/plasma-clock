#include "views/weather/daily_forecast.h"
#include <graphics/font.h>
#include <service/owm/weather.h>
#include <service/owm/weather_icons.h>

class DailyForecastView::Column: public Composite {
public:
    Column() {
        wants_clear_surface = true;

        icon = new AniSpriteView();
        icon->x_offset = 0;
        icon->width = 16;

        day_temp = new StringScroll(find_font(FONT_STYLE_UI_TEXT));
        day_temp->stopped = true;
        day_temp->set_y_position(0);
        day_temp->x_offset = 17;

        night_temp = new StringScroll(find_font(FONT_STYLE_UI_TEXT));
        night_temp->stopped = true;
        night_temp->set_y_position(8);
        night_temp->x_offset = 17;
        night_temp->gray = true;
        
        date = new StringScroll(find_font(FONT_STYLE_HUD_DIGITS));
        date->render_mode = TEXT_OUTLINED | TEXT_NO_BACKGROUND;
        date->stopped = true;
        date->set_y_position(11);
        date->x_offset = 9;

        add_composable(icon);
        add_composable(date);
        add_composable(day_temp);
        add_composable(night_temp);
    }

    void set_forecast(const forecast_weather_t * forecast) {
        if(forecast == nullptr) return;

        icon->set_sprite(sprite_from_conditions(forecast->conditions));
        
        char buf[16] = { 0 };
        const char fmt_temp[] = "%.0f°";
        if(forecast->day_temperature_kelvin > 0) {
            snprintf(buf, 15, fmt_temp, convert_temperature(KELVIN, forecast->day_temperature_kelvin));
            day_temp->set_string(buf);
        } else {
            day_temp->set_string("--°");
        }

        if(forecast->night_temperature_kelvin > 0) {
            snprintf(buf, 15, fmt_temp, convert_temperature(KELVIN, forecast->night_temperature_kelvin));
            night_temp->set_string(buf);
        } else {
            night_temp->set_string("--°");
        }

        if(forecast->date.day > 0) {
            snprintf(buf, 15, "%02d", forecast->date.day);
            date->set_string(buf);
        } else {
            date->set_string("");
        }
    }

private:
    AniSpriteView * icon;
    StringScroll * day_temp;
    StringScroll * night_temp;
    StringScroll * date;
};

DailyForecastView::DailyForecastView(bool ctl) {
    controllable = ctl;
    offset = 0;
    columns = {};

    one_widget_width = 48;

    screen_width = DisplayFramebuffer::width;
    update_width();
}

void DailyForecastView::prepare() {
    update_data();
    Screen::prepare();
}

void DailyForecastView::update_data() {
    for(int i = 0; i < columns.size(); i++) {
        columns[i]->set_forecast(weather_get_forecast(offset + i));
    }
}

void DailyForecastView::render(FantaManipulator *fb) {
    Screen::render(fb);

    // Animation looks like shite, let it just clip for now
    // if(fb->get_width() != screen_width) {
    //     screen_width = fb->get_width();
    //     need_width_update = true;
    // }
}

void DailyForecastView::update_width() {
    int x_offs = 0;
    one_widget_width = 48;
    total_per_screen = screen_width / one_widget_width;
    one_widget_width = screen_width / total_per_screen;
    while(total_per_screen > columns.size()) {
        Column * c = new Column();
        add_composable(c);
        columns.push_back(c);
    }

    for(int i = 0; i < total_per_screen; i++) {
        Column * c = columns[i];
        c->width = one_widget_width;
        c->x_offset = x_offs;
        c->hidden = false;
        x_offs += one_widget_width;
    }

    if(total_per_screen < columns.size()) {
        for(int i = total_per_screen - 1; i < columns.size(); i++) {
            columns[i]->hidden = true;
        }
    }

    need_width_update = false;
}

void DailyForecastView::step() {
    Screen::step();
    
    if(need_width_update) {
        offset = 0;
        update_width();
        update_data();
    }

    if(controllable) {
        if(hid_test_key_state_repetition(KEY_RIGHT)) {
            offset += total_per_screen;
            if((offset + 1) >= FORECAST_WEATHER_DAYS) {
                offset = 0;
            }
            update_data();
        }
    }
}
