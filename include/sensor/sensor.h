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

    SENSOR_ID_MAX
} sensor_id_t;

class PoolableSensor {
public:
    /// @brief Prepare the sensor for read operations
    /// @return Whether initialization has succeeded
    virtual bool initialize();
    /// @brief Release resources associated with the sensor
    virtual bool teardown();
    /// @brief Get the current sensor reading
    /// @param result Current sensor reading. Meaning depends on the specific sensor that was queried.
    /// @return Whether the reading succeeded and the `result` variable contains a valid value.
    virtual bool poll(int * result);
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

class SensorPool {
public:
    SensorPool();
    ~SensorPool();
    bool add(sensor_id_t, PoolableSensor *, TickType_t update_interval);
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