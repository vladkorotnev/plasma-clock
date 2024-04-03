#include <sensor/light.h>
#include <Arduino.h>

AmbientLightSensor::AmbientLightSensor(gpio_num_t pin) {
    active_pin = pin;
    pinMode(pin, ANALOG);
}

int AmbientLightSensor::read() {
    return analogRead(active_pin);
}

bool AmbientLightSensor::initialize() { return true; }
bool AmbientLightSensor::teardown() { return true; }

bool AmbientLightSensor::poll(int* result) {
    if(result == nullptr) return false;
    
    *result = read();
    return true;
}