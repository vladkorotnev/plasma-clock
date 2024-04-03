#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#pragma once

typedef enum sensor_id {
    SENSOR_ID_MOTION,

    SENSOR_ID_AMBIENT_TEMPERATURE,
    SENSOR_ID_AMBIENT_HUMIDITY,
    SENSOR_ID_AMBIENT_LIGHT,

    VIRT_SENSOR_ID_WEATHER_CONDITIONS,
    VIRT_SENSOR_ID_WEATHER_TEMPERATURE,
    VIRT_SENSOR_ID_WEATHER_HUMIDITY,
    VIRT_SENSOR_ID_WEATHER_PRESSURE,
    VIRT_SENSOR_ID_NETDEV_PRESENCE,

    SENSOR_ID_MAX
} sensor_id_t;

class PoolableSensor {
public:
    virtual bool initialize();
    virtual bool teardown();
    virtual bool poll(int * result);
};

typedef struct sensor_info {
    TickType_t update_interval;
    TickType_t last_read;
    PoolableSensor * handler;
    int last_result;
} sensor_info_t;

class SensorPool {
public:
    SensorPool();
    ~SensorPool();
    bool add(sensor_id_t, PoolableSensor *, TickType_t update_interval);
    void remove(sensor_id_t);
    sensor_info_t* get_info(sensor_id_t);
    void poll();
    TickType_t minimum_update_interval();

private:
    TaskHandle_t hTask;
    TickType_t min_interval;
    sensor_info_t* sensors[SENSOR_ID_MAX];
};