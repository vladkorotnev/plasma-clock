#include <views/idle_screens/indoor_view.h>
#include <rsrc/common_icons.h>
#include <service/owm/weather.h>
#include <fonts.h>

AirSensorView::AirSensorView(SensorPool *s, sensor_id_t t, sensor_id_t h, const sprite_t * ic) {
    sensors = s;
    value_font = &keyrus0816_font;
    temperature_id = t;
    humidity_id = h;
    icon = ic;
}

void AirSensorView::render(FantaManipulator *fb) {
    fb->put_sprite(icon, 0, 0);

    uint8_t hum_left_margin = 0;
    sensor_info_t * humidity = sensors->get_info(humidity_id);
    if(humidity != nullptr && humidity->last_read != 0) {
        char buf[8];
        snprintf(buf, 8, "%u%%", humidity->last_result/100);
        hum_left_margin = measure_string_width(value_font, buf) + value_font->width/2;
        fb->put_string(value_font, buf, fb->get_width() - hum_left_margin, 0);
    } else {
        hum_left_margin = 3*value_font->width + value_font->width/2;
        fb->put_string(value_font, "--%", fb->get_width() - hum_left_margin, 0);
    }

    sensor_info_t * temperature = sensors->get_info(temperature_id);
    if(temperature != nullptr && temperature->last_read != 0) {
        char buf[8];
        float converted = convert_temperature(CELSIUS, temperature->last_result / 100.0);
        snprintf(buf, 8, "%.1f°%c", converted, preferred_temperature_unit());
        uint8_t t_width = measure_string_width(value_font, buf);
        uint8_t t_left = (fb->get_width() - hum_left_margin + 16) / 2 - t_width / 2;
        fb->put_string(value_font, buf, t_left, 0);
    } else {
        uint8_t t_width = value_font->width*6;
        uint8_t t_left = (fb->get_width() - hum_left_margin + 16) / 2 - t_width / 2;
        fb->put_string(value_font, "--.-° ", t_left, 0);
    }
}

int AirSensorView::desired_display_time() {
    if(sensors->exists(humidity_id) || sensors->exists(temperature_id)) {
        return DISP_TIME_NO_OVERRIDE;
    } else {
        return DISP_TIME_DONT_SHOW;
    }
}

#if HAS(TEMP_SENSOR)
static const uint8_t icon_home_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x00, 0x00, 0x00, 0x00, 0x19, 0x80, 0x1a, 0x40, 0x15, 0xa0, 0x0b, 0xd0, 0x17, 0xe8, 0x2f, 0xf4, 
    0x5f, 0xfa, 0x1f, 0xf8, 0x11, 0x88, 0x11, 0x88, 0x11, 0x88, 0x11, 0xf8, 0x11, 0xf8, 0x00, 0x00
};

static const sprite_t icon_home_1616 = {
    .width = 16,
    .height = 16,
    .data = icon_home_data,
    .mask = nullptr
};

IndoorView::IndoorView(SensorPool *s) : AirSensorView(
    s,
    SENSOR_ID_AMBIENT_TEMPERATURE,
    SENSOR_ID_AMBIENT_HUMIDITY,
    &icon_home_1616
) {
}
#endif

#if HAS(SWITCHBOT_METER_INTEGRATION)
WoSensorView::WoSensorView(SensorPool *s) : AirSensorView(
    s,
    SENSOR_ID_SWITCHBOT_INDOOR_TEMPERATURE,
    SENSOR_ID_SWITCHBOT_INDOOR_HUMIDITY,
    &icon_thermo_1616
) {
}
#endif