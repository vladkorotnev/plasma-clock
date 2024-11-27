#pragma once
#include "keys.h"
#include <vector>

// @warning must match definitions of IRRecv library
typedef enum infrared_protocol {
    IRPROTO_RC5 = 1,
    IRPROTO_RC6 = 2,
    IRPROTO_NEC = 3,
    IRPROTO_SONY = 4
} infrared_protocol_t;

typedef struct infrared_identifier {
    infrared_protocol_t protocol;
    uint64_t value;
    uint32_t address;
    uint32_t command;
} infrared_identifier_t;

typedef std::vector<std::pair<const infrared_identifier_t, const key_id_t>> infrared_definition_t;
void infrared_start();
