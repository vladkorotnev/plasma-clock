typedef enum device_state {
    STATE_BOOT,
    STATE_IDLE,

    STATE_OTAFVU,
} device_state_t;

void change_state(device_state_t);