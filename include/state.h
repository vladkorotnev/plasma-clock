typedef enum device_state {
    STATE_BOOT,
    STATE_MENU,
    STATE_IDLE,
    STATE_ALARMING,

    STATE_OTAFVU,
} device_state_t;

void change_state(device_state_t);
void push_state(device_state_t next);
void pop_state(device_state_t expected);
