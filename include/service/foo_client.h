#pragma once
#include <freertos/FreeRTOS.h>

// Output option for foo_controlserver:
// %artist%|%title%
// Also set the main delimiter to |

#define FOO_SEPARATOR '|'

void foo_client_begin();

bool foo_is_playing();
void foo_get_title(char *, size_t);
void foo_get_artist(char *, size_t);
TickType_t foo_last_recv();
