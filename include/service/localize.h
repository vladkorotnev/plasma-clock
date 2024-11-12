#pragma once
#include <string>
#include <sound/yukkuri.h>
#include <service/time.h>

typedef enum display_language {
    DSPL_LANG_INVALID = -1,
    
    DSPL_LANG_EN = 0,
    DSPL_LANG_RU = 1,
    DSPL_LANG_JA = 2
} display_language_t;

typedef enum spoken_language {
    TTS_LANG_EN = 0,
    TTS_LANG_RU = 1,
    TTS_LANG_JA = 2,
} spoken_language_t;

display_language_t active_display_language();
const char * localized_string(const char* key);
const char * day_letter(int day);

spoken_language_t active_tts_language();
YukkuriUtterance localized_utterance_for_time(tk_time_of_day_t time, spoken_language_t lang = active_tts_language());
YukkuriUtterance localized_utterance_for_date(const tk_date_t * date, spoken_language_t lang = active_tts_language());
