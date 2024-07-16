#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#pragma once

typedef enum sensor_id {
    /// @brief A motion sensor, e.g. PIR
    SENSOR_ID_MOTION,

    /// @brief An ambient air temperature sensor
    SENSOR_ID_AMBIENT_TEMPERATURE,
    /// @brief An ambient air humidity sensor
    SENSOR_ID_AMBIENT_HUMIDITY,
    /// @brief An ambient light intensity sensor
    SENSOR_ID_AMBIENT_LIGHT,

    /// @brief A sensor using Switchbot Meter broadcasts for temperature measurement
    SENSOR_ID_SWITCHBOT_INDOOR_TEMPERATURE,
    /// @brief A sensor using Switchbot Meter broadcasts for humidity measurement
    SENSOR_ID_SWITCHBOT_INDOOR_HUMIDITY,

    /// @brief A virtual sensor indicating whether the PMU has been startled (brought out of the low power mode) recently
    VIRTSENSOR_ID_PMU_STARTLED,
    /// @brief A virtual sensor indicating whether the HID has been startled recently
    VIRTSENSOR_ID_HID_STARTLED,

    /// @brief A virtual sensor indicating the current wireless signal strength, or a value above 0 if disconnected
    VIRTSENSOR_ID_WIRELESS_RSSI,

    SENSOR_ID_MAX
} sensor_id_t;

/// @brief Poll the sensor every time along with some other one
const TickType_t SENSOR_POLLRATE_ASARP = 0;

class PoolableSensor {
public:
    /// @brief Prepare the sensor for read operations
    /// @return Whether initialization has succeeded
    virtual bool initialize() { return true; }
    /// @brief Release resources associated with the sensor
    virtual bool teardown() { return true; }
    /// @brief Get the current sensor reading
    /// @param result Current sensor reading. Meaning depends on the specific sensor that was queried.
    /// @return Whether the reading succeeded and the `result` variable contains a valid value.
    virtual bool poll(int * result) { return false; }
};

typedef struct sensor_info {
    /// @brief Interval to update the sensor at (not guaranteed to be exact)
    TickType_t update_interval;
    /// @brief Last time a value was read from the sensor
    TickType_t last_read;
    /// @brief Pointer to the sensor instance that handles this sensor
    PoolableSensor * handler;
    /// @brief Last value read from the sensor
    int last_result;
} sensor_info_t;

/// @brief A sensor providing an arbitrary constant value
class FauxSensor: public PoolableSensor {
public:
    int value = 0;
    FauxSensor(int initialValue) {
        value = initialValue;
    }

    bool poll(int * result) {
        *result = value;
        return true;
    }
};

/// @brief A sensor providing a 1 when triggered, and a 0 after a certain time
class TimerSensor: public PoolableSensor {
public:
    TickType_t duration;
    TimerSensor(TickType_t dur) {
        duration = dur;
        lastTrigger = xTaskGetTickCount();
    }

    bool poll(int * result) {
        *result = (xTaskGetTickCount() - lastTrigger >= duration) ? 0 : 1;
        return true;
    }

    void trigger() {
        lastTrigger = xTaskGetTickCount();
    }
private:
    TickType_t lastTrigger;
};

class SensorPool {
public:
    SensorPool();
    ~SensorPool();
    bool add(sensor_id_t, PoolableSensor *, TickType_t update_interval);
    void short_circuit(sensor_id_t faux, sensor_id_t origin);
    bool exists(sensor_id_t);
    void remove(sensor_id_t);
    sensor_info_t* get_info(sensor_id_t);
    /// @brief Poll the sensors in the pool which have their update intervals expired
    void poll();
    TickType_t minimum_update_interval();

private:
    TaskHandle_t hTask;
    TickType_t min_interval;
    sensor_info_t* sensors[SENSOR_ID_MAX];
};

class LoopSensor: public PoolableSensor {
public:
    LoopSensor(SensorPool* parent, sensor_id_t loop_to) {
        pool = parent;
        actual_id = loop_to;
    }

    // Not overriding teardown here, that will be taken care of by the origin sensor entry

    bool initialize() {
        return pool->exists(actual_id);
    }

    bool poll(int * result) {
        sensor_info_t * info = pool->get_info(actual_id);
        *result = info->last_result;
        return true;
    }

private:
    SensorPool *pool;
    sensor_id_t actual_id;
};