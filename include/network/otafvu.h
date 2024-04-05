#pragma once
#include <plasma/framebuffer.h>
#include <console.h>
#include <ArduinoOTA.h>

class OTAFVUManager {
public:
    OTAFVUManager(Console*);
    ~OTAFVUManager();

    void task();
private:
    Console *con;
    void get_ready();
    void on_error(ota_error_t);
    void shut_up_and_explode();
    void on_progress(unsigned int, unsigned int);
    TaskHandle_t hTask;
};
