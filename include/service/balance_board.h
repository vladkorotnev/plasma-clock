#pragma once
#include <sensor/sensor.h>
#include <device_config.h>

#if HAS(BALANCE_BOARD_INTEGRATION)

typedef enum BalanceBoardState {
    BB_IDLE,
    BB_SCANNING,
    BB_CONNECTED
} balance_board_state_t;

void balance_board_start(SensorPool*);
void balance_board_scan(bool);
balance_board_state_t balance_board_state();
void balance_board_zero();
#endif