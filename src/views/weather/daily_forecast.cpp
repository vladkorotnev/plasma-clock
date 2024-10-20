#include "views/weather/daily_forecast.h"
#include <fonts.h>
#include <service/owm/weather.h>
#include <service/owm/weather_icons.h>

class DailyForecastView::Column: public Composite {
public:
    Column() {
        icon = new AniSpriteView();
        icon->x_offset = 0;
        icon->width = 16;

        day_temp = new StringScroll(&keyrus0808_font);
        day_temp->stopped = true;
        day_temp->set_y_position(0);
        day_temp->x_offset = 17;

        night_temp = new StringScroll(&keyrus0808_font);
        night_temp->stopped = true;
        night_temp->set_y_position(8);
        night_temp->x_offset = 17;
        night_temp->gray = true;
        
        date = new StringScroll(&fps_counter_font);
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
        const char fmt_temp[] = "%.0f\370";
        if(forecast->day_temperature_kelvin > 0) {
            snprintf(buf, 15, fmt_temp, convert_temperature(KELVIN, forecast->day_temperature_kelvin));
            day_temp->set_string(buf);
        } else {
            day_temp->set_string("--\370");
        }

        if(forecast->night_temperature_kelvin > 0) {
            snprintf(buf, 15, fmt_temp, convert_temperature(KELVIN, forecast->night_temperature_kelvin));
            night_temp->set_string(buf);
        } else {
            night_temp->set_string("--\370");
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

    leftView = new Column();
    leftView->width = 48;

    rightView = new Column();
    rightView->width = 48;
    rightView->x_offset = std::max(rightView->width + 4, HWCONF_DISPLAY_WIDTH_PX/2);

    add_composable(leftView);
    add_composable(rightView);
}

DailyForecastView::~DailyForecastView() {
    delete leftView;
    delete rightView;
}

void DailyForecastView::prepare() {
    leftView->set_forecast(weather_get_forecast(0));
    rightView->set_forecast(weather_get_forecast(1));
    Screen::prepare();
}

void DailyForecastView::step() {
    Screen::step();
    if(controllable) {
        if(hid_test_key_state_repetition(KEY_RIGHT)) {
            offset += 2;
            if((offset + 1) >= FORECAST_WEATHER_DAYS) {
                offset = 0;
            }
            leftView->set_forecast(weather_get_forecast(offset));
            rightView->set_forecast(weather_get_forecast(offset + 1));
        }
    }
}
