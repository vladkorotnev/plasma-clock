#pragma once
#include <device_config.h>
#include <views/common/view.h>
#include <graphics/fanta_manipulator.h>
#include <sensor/sensor.h>

class AirSensorView: public Screen {
public:
    AirSensorView(SensorPool*, sensor_id_t temp, sensor_id_t hum, const sprite_t * icon);
    void render(FantaManipulator*);
    int desired_display_time();

private:
    SensorPool * sensors;
    const font_definition_t * value_font;
    const sprite_t * icon;
    sensor_id_t humidity_id;
    sensor_id_t temperature_id;
};

#if HAS(TEMP_SENSOR)
class IndoorView: public AirSensorView {
public:
    IndoorView(SensorPool*);
};
#endif

#if HAS(SWITCHBOT_METER_INTEGRATION)
class WoSensorView: public AirSensorView {
public:
    WoSensorView(SensorPool*);
};
#endif