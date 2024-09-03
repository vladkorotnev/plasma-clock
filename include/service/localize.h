#pragma once
#include <string>

typedef enum display_language {
    DSPL_LANG_EN = 0,
    DSPL_LANG_RU = 1
} display_language_t;

typedef enum spoken_language {
    TTS_LANG_EN = 0,
    TTS_LANG_RU = 1,
    TTS_LANG_JA = 2,
} spoken_language_t;

display_language_t active_display_language();
const char * localized_string(const std::string key);
const char * day_letters();
