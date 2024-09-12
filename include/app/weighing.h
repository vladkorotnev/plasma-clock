#pragma once
#include <device_config.h>
#include <sensor/sensor.h>
#include <views/framework.h>
#include <views/menu/menu.h>

#if HAS(BALANCE_BOARD_INTEGRATION)
class AppShimWeighing: public Composite {
public:
    AppShimWeighing(SensorPool *);
    ~AppShimWeighing();

    void prepare();
    void step();

private:
    ViewMultiplexor * carousel;
    SensorPool * sensors;
    TickType_t lastActivity;
    class WeighingView;
    enum WeighingAppState {
        NEED_CONNECT,
        WAIT_CONNECT,
        WEIGHING
    };
    WeighingAppState curState;
    void update_state(transition_type_t);
};
#endif