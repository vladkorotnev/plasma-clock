#include <views/indoor_view.h>
#include <fonts.h>

static const uint8_t icon_home_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x00, 0x00, 0x00, 0x00, 0x19, 0x80, 0x1a, 0x40, 0x15, 0xa0, 0x0b, 0xd0, 0x17, 0xe8, 0x2f, 0xf4, 
    0x5f, 0xfa, 0x1f, 0xf8, 0x11, 0x88, 0x11, 0x88, 0x11, 0x88, 0x11, 0xf8, 0x11, 0xf8, 0x00, 0x00
};

static const sprite_t icon_home_1616 = {
    .width = 16,
    .height = 16,
    .data = icon_home_data
};

IndoorView::IndoorView(SensorPool *s) {
    sensors = s;
    value_font = &keyrus0816_font;
}

void IndoorView::render(FantaManipulator *fb) {
    fb->put_sprite(&icon_home_1616, 0, 0);

    uint8_t hum_left_margin = 0;
    sensor_info_t * humidity = sensors->get_info(SENSOR_ID_AMBIENT_HUMIDITY);
    if(humidity != nullptr && humidity->last_read != 0) {
        char buf[8];
        snprintf(buf, 8, "%u%%", humidity->last_result/100);
        hum_left_margin = measure_string_width(value_font, buf) + value_font->width/2;
        fb->put_string(value_font, buf, fb->get_width() - hum_left_margin, 0);
    } else {
        hum_left_margin = 3*value_font->width + value_font->width/2;
        fb->put_string(value_font, "--%", fb->get_width() - hum_left_margin, 0);
    }

    sensor_info_t * temperature = sensors->get_info(SENSOR_ID_AMBIENT_TEMPERATURE);
    if(temperature != nullptr && temperature->last_read != 0) {
        char buf[8];
        snprintf(buf, 8, "%i.%i\370C", temperature->last_result/100, (temperature->last_result < 0 ? -1 : 1) * (temperature->last_result%100)/10);
        uint8_t t_width = measure_string_width(value_font, buf);
        uint8_t t_left = (fb->get_width() - hum_left_margin + 16) / 2 - t_width / 2;
        fb->put_string(value_font, buf, t_left, 0);
    } else {
        uint8_t t_width = value_font->width*6;
        uint8_t t_left = (fb->get_width() - hum_left_margin + 16) / 2 - t_width / 2;
        fb->put_string(value_font, "--.-\370C", t_left, 0);
    }
}

int IndoorView::desired_display_time() {
    if(sensors->exists(SENSOR_ID_AMBIENT_HUMIDITY) || sensors->exists(SENSOR_ID_AMBIENT_TEMPERATURE)) {
        return DISP_TIME_NO_OVERRIDE;
    } else {
        return DISP_TIME_DONT_SHOW;
    }
}