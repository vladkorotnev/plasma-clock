#include <sensor/motion.h>
#include <Arduino.h>

MotionSensor::MotionSensor(gpio_num_t pin) {
    active_pin = pin;
    pinMode(pin, INPUT_PULLDOWN);
}

bool MotionSensor::read() {
    return digitalRead(active_pin);
}

bool MotionSensor::initialize() { return true; }
bool MotionSensor::teardown() { return true; }

bool MotionSensor::poll(int* result) {
    if(result == nullptr) return false;
    
    *result = read() ? 1 : 0;
    return true;
}