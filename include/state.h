#include <views/transitions/transitions.h>

typedef enum device_state {
    STATE_BOOT,
    STATE_MENU,
    STATE_ALARM_EDITOR,
    STATE_IDLE,
    STATE_ALARMING,

    STATE_OTAFVU,
} device_state_t;

void change_state(device_state_t, transition_type_t = TRANSITION_WIPE);
void push_state(device_state_t next, transition_type_t = TRANSITION_WIPE);
void pop_state(device_state_t expected, transition_type_t = TRANSITION_WIPE);
