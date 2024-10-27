#pragma once
#include <views/transitions/transitions.h>
#include <device_config.h>

typedef enum device_state {
    STATE_BOOT,
    STATE_RESTART,

    STATE_MENU,
    STATE_ALARM_EDITOR,
    STATE_TIMER_EDITOR,
    STATE_IDLE,
    STATE_ALARMING,
    STATE_STOPWATCH,
    STATE_WEATHER,
    STATE_MUSICBOX,
#if HAS(BALANCE_BOARD_INTEGRATION)
    STATE_WEIGHING,
#endif
#if HAS(HTTPFVU)
    STATE_HTTPFVU,
#endif
    STATE_NEW_YEAR,
    STATE_OTAFVU,
#if HAS(PLAYGROUND)
    STATE_PLAYGROUND,
#endif
} device_state_t;

void change_state(device_state_t, transition_type_t = TRANSITION_WIPE);
void push_state(device_state_t next, transition_type_t = TRANSITION_WIPE);
void pop_state(device_state_t expected, transition_type_t = TRANSITION_WIPE);
