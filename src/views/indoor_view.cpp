#include <views/indoor_view.h>
#include <fonts.h>

static const uint8_t icon_home_data[] = {
    0x00, 0x00, 0x00, 0x30, 0x01, 0x30, 0x02, 0xB0, 0x05, 0x70, 0x09, 0x30, 0x10, 0x10, 0x3F, 0xF8, 0x50, 0x14, 0x17, 0x90, 0x14, 0x90, 0x14, 0x90, 0x15, 0x90, 0x14, 0x90, 0x1F, 0xF0, 0x00, 0x00, 0x00
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
        snprintf(buf, 8, "%u.%u\370C", temperature->last_result/100, (temperature->last_result%100)/10);
        uint8_t t_width = measure_string_width(value_font, buf);
        uint8_t t_left = (fb->get_width() - hum_left_margin + 16) / 2 - t_width / 2;
        fb->put_string(value_font, buf, t_left, 0);
    } else {
        uint8_t t_width = value_font->width*6;
        uint8_t t_left = (fb->get_width() - hum_left_margin + 16) / 2 - t_width / 2;
        fb->put_string(value_font, "--.-\370C", t_left, 0);
    }
}