#include <sensor/sensor.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>

static char LOG_TAG[] = "SENS";

extern "C" void SensorPoolTaskFunc( void * pvParameter );

void SensorPoolTaskFunc( void * pvParameter )
{
    SensorPool * that = static_cast<SensorPool*> ( pvParameter );

    while(1) {
        that->poll();
        vTaskDelay(that->minimum_update_interval());
    }
}

SensorPool::SensorPool() {
    for(int i = 0; i < SENSOR_ID_MAX; i++) sensors[i] = nullptr;
    min_interval = pdMS_TO_TICKS(5000);

    if(xTaskCreate(
        SensorPoolTaskFunc,
        "SENS",
        4096,
        this,
        10,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

SensorPool::~SensorPool() {
    for(int i = 0; i < SENSOR_ID_MAX; i++) remove((sensor_id_t)i);

    if(hTask != NULL) {
        vTaskDelete(hTask);
        hTask = NULL;
    }
}

void SensorPool::remove(sensor_id_t id) {
    if(sensors[id] != nullptr) {
        sensors[id]->handler->teardown();
        free(sensors[id]);
        sensors[id] = nullptr;
    }
}

bool SensorPool::add(sensor_id_t id, PoolableSensor * sensor, TickType_t interval) {
    if(sensors[id] != nullptr) {
        ESP_LOGE(LOG_TAG, "Sensor with id %i already exists!!", id);
        return false;
    }

    if(!sensor->initialize()) {
        ESP_LOGE(LOG_TAG, "Not adding sensor (id %i): initialization failed", id);
        return false;
    }

    sensor_info * info = (sensor_info*) malloc(sizeof(sensor_info));
    info->last_read = 0;
    info->update_interval = interval;
    info->handler = sensor;
    info->last_result = 0;

    ESP_LOGV(LOG_TAG, "Adding sensor with id %i", id);
    sensors[id] = info;
    if(interval/2 < min_interval) min_interval = interval/2;
    
    return true;
}

bool SensorPool::exists(sensor_id_t id) {
    return sensors[id] != nullptr;
}

TickType_t SensorPool::minimum_update_interval() {
    return min_interval;
}

sensor_info_t* SensorPool::get_info(sensor_id_t id) {
    return sensors[id];
}

void SensorPool::poll() {
    TickType_t now = xTaskGetTickCount();

    for(int i = 0; i < SENSOR_ID_MAX; i++) {
        sensor_info_t * sensor = get_info((sensor_id_t) i);
        if(sensor == nullptr) continue;
        if(now - sensor->last_read >= sensor->update_interval) {
            int value = 0;
            if(sensor->handler->poll(&value)) {
                sensor->last_result = value;
                sensor->last_read = xTaskGetTickCount();
#ifdef SENSOR_SPAM_LOGS
                ESP_LOGI(LOG_TAG, "Sensor %i new value: %i", i, value);
#endif
            } else {
                ESP_LOGE(LOG_TAG, "Poll of sensor %i failed", i);
            }
        }
    }
}