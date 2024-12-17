#include <service/localize.h>
#include <service/prefs.h>
#include <service/disk.h>
#include <device_config.h>
#include <map>
#include <ArduinoJson.h>

static char LOG_TAG[] = "LOCA";
#define LANG_DIR "/lang/"

display_language_t active_display_language() {
    return (display_language_t) prefs_get_int(PREFS_KEY_DISP_LANGUAGE);
}

spoken_language_t active_tts_language() {
    return (spoken_language_t) prefs_get_int(PREFS_KEY_TTS_LANGUAGE);
}

const char * day_letter(int day) {
    static const char * const en_letters[14] = {
        "S", "M", "T", "W", "T", "F", "S"
    };

    switch(active_display_language()) {
        case DSPL_LANG_RU:
            static const char * const ru_letters[7] = {
                "В", "П", "В", "С", "Ч", "П", "С"
            };
            return ru_letters[day];

        case DSPL_LANG_JA:
            static const char * const ja_letters[7] = {
                "日", "月", "火", "水", "木", "金", "土"
            };
            return ja_letters[day];

        case DSPL_LANG_EN:
        default:
            return en_letters[day];
    }
}

static display_language_t lang_map_language = DSPL_LANG_INVALID;
static EXT_RAM_ATTR std::map<const std::string, char*> lang_map = {};

static void _load_lang_map_if_needed() {
    display_language_t lang =  active_display_language();
    if(lang_map_language == lang) return;

    const char * filename = nullptr;
    switch(lang) {
        case DSPL_LANG_RU:
            filename = LANG_DIR "ru.lang";
            break;
        case DSPL_LANG_JA:
            filename = LANG_DIR "ja.lang";
            break;
        case DSPL_LANG_EN:
        default:
            filename = LANG_DIR "en.lang";
            break;
    }

    File f = open_file(filename);
    if(!f) {
        ESP_LOGE(LOG_TAG, "Loading %s failed! No such file? Pretending nothing happened.", filename);
        lang_map_language = lang;
        return;
    }

    JsonDocument content;
    DeserializationError error = deserializeJson(content, f);
    if(error) {
        ESP_LOGE(LOG_TAG, "Parsing %s failed: %s", filename, error.c_str());
        lang_map_language = lang;
        return;
    }

    JsonObject root = content.as<JsonObject>();

    for (auto kv : lang_map) {
        free(kv.second);
    }

    lang_map.clear();
    int i = 0;

    for (JsonPair kv : root) {
        const char * val = kv.value().as<const char *>();
        char * dst = (char*)malloc(strlen(val) + 1);
        strcpy(dst, val);
        lang_map[kv.key().c_str()] = dst;
        i++;
    }

    lang_map_language = lang;
    ESP_LOGI(LOG_TAG, "Loaded language ID=%i, Entries: %i", lang_map_language, i);
}

const char * localized_string(const char* key) {
    _load_lang_map_if_needed();

    if(lang_map.count(key)) {
        return lang_map.at(key);
    } else {
        return key;
    }
}


typedef enum tts_number_kind {
    TTSNUM_NOMINATIVE, // default
    TTSNUM_NOMINATIVE_M, // один, два, три...
    TTSNUM_NOMINATIVE_F, // одна, две, три...
    TTSNUM_ORDINAL // первое, второе, третье...
} tts_number_kind_t;

const std::string _tts_number_in_language(int number, spoken_language_t lang, tts_number_kind_t kind) {
    if(number > 999) return "";

    const std::string * minus;
    const std::string * numbers;

    switch(lang) {
        case TTS_LANG_EN:
            {
                static const std::string en_minus = "ma'ina_su/";
                minus = &en_minus;
                static const std::string en_numbers[] = {
                    "ziro",
                    "wan",
                    "tuu",
                    "suri'-",
                    "fo-",
                    "fai_fu",
                    "si_ku_su",
                    "se'vunn",
                    "ei_tu",
                    "na'inn",
                    "te'nn",
                    "ire'vunn",
                    "tue'ru_fu",
                    "_fioruti'-n",
                    "foruti'-n",
                    "fi_futi'-n",
                    "si_ku_suti'-n",
                    "se'vunnti'-n",
                    "eiti'-n",
                    "na'inti'-n",
                    "tue'nnti", // 20
                    "sho'-ti", // 30
                    "fo'-ti", // 40
                    "fi'_futi", // 50,
                    "si'_ku_suti", // 60
                    "se'vunnti", // 70
                    "e'iti", // 80
                    "na'innti", // 90
                    "ha'nndure_tu" // 100
                };
                static const std::string en_ordinals[] = {
                    "ziro",
                    "fyo'-_su_tu",
                    "se'kann_tu",
                    "sho'-_tu",
                    "fo-_tu",
                    "fi'_fu_tu",
                    "si'_ku_su_tu",
                    "se'vunn_tu",
                    "ei_tu",
                    "na'inn_tu",
                    "te'nn_tu",
                    "ire'vunn_tu",
                    "tue'ru_fu_tu",
                    "_fioruti'-n_tu",
                    "foruti'-n_tu",
                    "fi_futi'-n_tu",
                    "si_ku_suti'-n_tu",
                    "se'vunnti'-n_tu",
                    "eiti'-n_tu",
                    "na'inti'-n_tu",
                    "tue'nnti", // 20
                    "_sio'-ti", // 30
                    "fo'-ti", // 40
                    "fi'_futi", // 50,
                    "si'_ku_suti", // 60
                    "se'vunnti", // 70
                    "e'iti", // 80
                    "na'innti", // 90
                    "ha'nndure_tu" // 100
                };
                numbers = (kind == TTSNUM_ORDINAL) ? en_ordinals : en_numbers;
            }
            break;

        case TTS_LANG_RU:
            {
                static const std::string ru_minus = "mi'nu_su/";
                minus = &ru_minus;
                static const std::string ru_numbers_m[] = {
                    "no'ri",
                    "oji'nn",
                    "dua'",
                    "turi'",
                    "cheti'ri",
                    "pya'_tu",
                    "she'_si_ti",
                    "she'nn",
                    "wo'shenn",
                    "zye'bya_ti",
                    "zye'sya_ti",
                    "oji'nnaza_ti",
                    "duena'za_ti",
                    "turina'za_ti",
                    "cheti'runaza_ti",
                    "pya_tuna'za_ti",
                    "she_suna'za_ti",
                    "syemuna'za_ti",
                    "vosyemuna'za_ti",
                    "zyebyatuna'za_ti",
                    "dua'za_ti",
                    "turi'sa_ti", // 30
                    "so'ra_ku", // 40
                    "pi_tujyesia'_tu", // 50,
                    "si'_sujyesia'_tu", // 60
                    "sye'nnjyesia_ti", // 70
                    "vo'syennjyese_ti", // 80
                    "jyebyano'_suta", // 90
                    "_suto'" // 100
                };
                static const std::string ru_numbers_f[] = {
                    "no'ri",
                    "o_tuna'",
                    "duye'",
                    "turi'",
                    "cheti'ri",
                    "pya'_tu",
                    "she'_si_ti",
                    "she'nn",
                    "wo'shenn",
                    "zye'bya_ti",
                    "zye'sya_ti",
                    "oji'nnaza_ti",
                    "duena'za_ti",
                    "turina'za_ti",
                    "cheti'runaza_ti",
                    "pya_tuna'za_ti",
                    "she_suna'za_ti",
                    "syemuna'za_ti",
                    "vosyemuna'za_ti",
                    "zyebyatuna'za_ti",
                    "dua'za_ti",
                    "turi'sa_ti", // 30
                    "so'ra_ku", // 40
                    "pi_tujyesia'_tu", // 50,
                    "si'_sujyesia'_tu", // 60
                    "sye'nnjyesia_ti", // 70
                    "vo'syennjyese_ti", // 80
                    "jyebyano'_suta", // 90
                    "_suto'" // 100
                };
                static const std::string ru_ordinals[] = {
                    "nurivo'ie",
                    "pye'ruvoie",
                    "_futoro'ie",
                    "_turye'tie",
                    "chi_tubyo'rutoie",
                    "pya'toie",
                    "she_suto'ie",
                    "shejimo'ie",
                    "wo_shimo'ie",
                    "zyebya'toie",
                    "zyesya'toie",
                    "oji'nnazatoie",
                    "duena'zatoie",
                    "turina'zatoie",
                    "cheti'runazatoie",
                    "pya_tuna'zatoie",
                    "she_suna'zatoie",
                    "syemuna'zatoie",
                    "vosyemuna'zatoie",
                    "zyebyatuna'zatoie",
                    "dua'za", // 20
                    "turi'sa", // 30
                    "so'ra", // 40
                    "pi_tujyesia'", // 50,
                    "si'_sujyesia'", // 60
                    "sye'nnjyesia", // 70
                    "vo'syennjyese", // 80
                    "jyebyano'_suta", // 90
                    "_suto'" // 100
                };
                switch(kind) {
                    case TTSNUM_NOMINATIVE_F:
                        numbers = ru_numbers_f;
                        break;
                    case TTSNUM_ORDINAL:
                        numbers = ru_ordinals;
                        break;
                    case TTSNUM_NOMINATIVE:
                    case TTSNUM_NOMINATIVE_M:
                    default:
                        numbers = ru_numbers_m;
                        break;
                }
            }
            break;

        default:
        case TTS_LANG_JA:
            return "";
    }
    
    std::string acc = "";
    if(number < 0) {
        acc += *minus;
        number = -1*number;
    }
    int hundreds = (number / 100) % 10;
    if(hundreds > 1) {
        acc += numbers[hundreds] + '/' + numbers[28] /* "hundred" */;
    }
    int teens = (number % 100);
    if(teens <= 20) { // cases 0..20
        // disallow e.g. "one hundred zero"
        if((hundreds == 0 && teens == 0) || teens > 0) {
            if(acc.length() > 0) acc += '/';
            if(hundreds > 0 && lang == TTS_LANG_EN) {
                acc += "e'nn_tu/"; // "and"
            }
            acc += numbers[teens];
            if(teens == 20 && kind == TTSNUM_ORDINAL) {
                // twentieTH / двадцаТОЕ
                acc += (lang == TTS_LANG_EN ? "_tu" : "toie");
            }
        } 
    } else { // cases 21..
        if(acc.length() > 0) acc += '/';
        int decades = (number / 10) % 10;
        int ones = number % 10;
        acc += numbers[(decades - 2) + 20];
        if(ones > 0) {
            if(kind == TTSNUM_ORDINAL && lang == TTS_LANG_RU) {
                if(decades == 2 || decades == 3 || decades == 7 || decades == 8) {
                    // двадцаТЬ нное, тридцаТЬ нное, семьдесять нное, восемьдесять нное
                    acc += "_ti";
                }
                else if (decades == 4) {
                    // сороК нное
                    acc += "_ku";
                }
                else if (decades == 5 || decades == 6) {
                    // пятьдесяТ шестьдесяТ нное
                    acc += "_tu";
                }
            }
            acc += '/';
            acc += numbers[ones];
        } else if (kind == TTSNUM_ORDINAL) {
            if(lang == TTS_LANG_RU) {
                if(decades == 2 || decades == 3 || decades == 7 || decades == 8 || decades == 5 || decades == 6) {
                    // двадцаТое, тридцаТое, семьдесятое, восемьдесятое, пятьдесятое, шестьдесятое
                    acc += "toie";
                }
                else if (decades == 4) {
                    // сороКОВОЕ
                    acc += "kobo'ie";
                }
                else if (decades == 9) {
                    // девяностоЕ
                    acc += "ie";
                }
            } else {
                // thirtieTH, fortieTH...
                acc += "_tu";
            }
        }
    }
    return acc;
}


static const char * _tts_days_en[] = {
    "sa'nndei", // Sun
    "ma'nndei", // Mon
    "chu'_sudei", // Tue
    "ue'nn_sudei", // Wed
    "syo'ru_sudei", // Thu
    "_fura'idei", // Fri
    "se'chudei", // Sat
};
static const char * _tts_days_ru[] = {
    "bo_sukurise'nye", // Sun
    "ponyeje'rini_ku", // Mon
    "futo'runi_ku", // Tue
    "_surida'", // Wed
    "che_tuve'ru_ku", // Thu
    "pya'_tunitsa", // Fri
    "subbo'ta", // Sat
};
static const char * _tts_days_ja[] = {
    "nitiyo-bi", // Sun
    "getuyo-bi", // Mon
    "kayo-bi", // Tue
    "suiyo-bi", // Wed
    "mokuyo-bi", // Thu
    "kinyo-bi", // Fri
    "doyo-bi", // Sat
};

static const char * _tts_months_en[] =  {
    "ja'nyuari", // Jan
    "fe'brueri", // Feb
    "ma'_ti", // Mar
    "e'_puriru", // Apr
    "me'i", // May
    "junn", // Jun
    "jurai", // Jul
    "o'-ga_su_tu", // Aug
    "se_putenba", // Sep
    "o_kuto'ba", // Oct
    "nove'nnba", // Nov
    "dise'nnba", // Dec
};
static const char * _tts_months_ru[] =  {
    "yannbarya'", // Jan
    "fevurarya'", // Feb
    "ma'ruta", // Mar
    "a_purye'rya", // Apr
    "ma'ya", // May
    "iyu'nya", // Jun
    "iyu'rya", // Jul
    "a'_fugu_suta", // Aug
    "syennchaburya'", // Sep
    "o_kutyaburya'", // Oct
    "nayeburya'", // Nov
    "jikaburya'", // Dec
};



YukkuriUtterance localized_utterance_for_time(tk_time_of_day_t _time, spoken_language_t lang) {
    tk_time_of_day_t time = _time; // local copy for 12h conversion
    
    // NB: This will be returning a static utterance. 
    // Assuming this is called roughly once an hour it's fine.
    // Otherwise need to change logic to allocate dynamically and release appropriately which is a PITA!
    static char hourBuf[128] = { 0 };
    bool is_12h = false;
    bool is_pm = false;

    if(!prefs_get_bool(PREFS_KEY_VOICE_24_HRS)) {
        is_12h = true;
        convert_to_12h(&time, &is_pm);
    }

    switch(lang) {
        case TTS_LANG_JA:
            if(time.hour == 12 && time.minute == 0) {
                snprintf(hourBuf, sizeof(hourBuf), "sho'-go/de_su,");
            } else {
                if(is_12h) {
                    if(time.minute == 0) {
                        snprintf(hourBuf, sizeof(hourBuf), "%s,<NUMK VAL=%i COUNTER=ji>/de_su,", is_pm ? "go'go" : "gozenn", time.hour);
                    } else {
                        snprintf(hourBuf, sizeof(hourBuf), "%s,<NUMK VAL=%i COUNTER=ji>,<NUMK VAL=%i COUNTER=funn>/de_su,", is_pm ? "go'go" : "gozenn", time.hour, time.minute);
                    }
                } else {
                    if(time.minute == 0) {
                        snprintf(hourBuf, sizeof(hourBuf), "<NUMK VAL=%i COUNTER=ji>,de_su,", time.hour);
                    } else {
                        snprintf(hourBuf, sizeof(hourBuf), "<NUMK VAL=%i COUNTER=ji>,<NUMK VAL=%i COUNTER=funn>/de_su,", time.hour, time.minute);
                    }
                }
            }
        break;

        case TTS_LANG_RU:
            if((time.hour == 12 && (!is_12h || is_pm)) && time.minute == 0) {
                strncpy(hourBuf, "po'rujieni,", sizeof(hourBuf));
            } else if((time.hour == 0 || (time.hour == 12 && !is_pm)) && time.minute == 0) {
                strncpy(hourBuf, "po'runo_chi,", sizeof(hourBuf));
            } else {
                std::string acc = _tts_number_in_language(time.hour, TTS_LANG_RU, TTSNUM_NOMINATIVE_M);
                int hr_ones = time.hour % 10;
                if(hr_ones == 0 || hr_ones >= 5 || ((time.hour / 10) % 10) == 1) {
                    acc += "/chiso'_fu";
                }
                else if(hr_ones == 1) {
                    acc += "/cha'_su";
                }
                else if(hr_ones >= 2) {
                    acc += "/chisa'";
                }
                
                if(time.minute == 0) {
                    if(is_12h) {
                        if(time.hour < 4 && !is_pm) {
                            acc += "/no'chi";
                        } else if(time.hour < 12 && !is_pm) {
                            acc += "/u_tura'";
                        } else if(time.hour <= 4 && is_pm) {
                            acc += "/dunya'";
                        } else if(time.hour >= 5 && is_pm) {
                            acc += "/bie'chira";
                        }
                    } else {
                        acc += "/ro'funo,";
                    }
                } else {
                    acc += ',';
                    acc += _tts_number_in_language(time.minute, TTS_LANG_RU, TTSNUM_NOMINATIVE_F);
                    int min_ones = time.minute % 10;
                    if(min_ones >= 5 || min_ones == 0 || ((time.minute / 10) % 10) == 1) {
                        acc += "/minu'_tu";
                    }
                    else if(min_ones == 1) {
                        acc += "/minu'ta";
                    }
                    else if(min_ones >= 2) {
                        acc += "/minu'ti";
                    }
                    acc += '.';
                }
                strncpy(hourBuf, acc.c_str(), sizeof(hourBuf));
            }
        break;

        case TTS_LANG_EN:
            if((time.hour == 12 && (!is_12h || is_pm)) && time.minute == 0) {
                strncpy(hourBuf, "nu'-n,", sizeof(hourBuf));
            } else if((time.hour == 0 || (time.hour == 12 && !is_pm)) && time.minute == 0) {
                strncpy(hourBuf, "mi'dunai_tu,", sizeof(hourBuf));
            } else {
                std::string acc = _tts_number_in_language(time.hour, TTS_LANG_EN, TTSNUM_NOMINATIVE);
                if(time.minute == 0) {
                    if(!is_12h) {
                        acc += "/o/_kura'_ku,";
                    } else {
                        acc += is_pm ? "/pi';e'n" : "/e'i;e'n";
                    }
                } else {
                    acc += ',';
                    acc += _tts_number_in_language(time.minute, TTS_LANG_EN, TTSNUM_NOMINATIVE);
                    if(is_12h) {
                        acc += is_pm ? "/pi';e'n" : "/e'i;e'n";
                    }
                    acc += '.';
                }
                strncpy(hourBuf, acc.c_str(), sizeof(hourBuf));
            }
        break;

        default: break;
    }

    return YukkuriUtterance(hourBuf);
}

YukkuriUtterance localized_utterance_for_date(const tk_date_t * date, spoken_language_t lang) {
    // NB: This will be returning a static utterance. 
    // Assuming this is called roughly once an hour it's fine.
    // Otherwise need to change logic to allocate dynamically and release appropriately which is a PITA!
    static char dateBuf[128] = {0};
    
    switch(lang) {
        case TTS_LANG_RU:
            {
                std::string acc = _tts_days_ru[date->dayOfWeek];
                acc += ',';
                acc += _tts_number_in_language(date->day, TTS_LANG_RU, TTSNUM_ORDINAL);
                acc += ';';
                acc += _tts_months_ru[date->month - 1];
                acc += ',';
                strncpy(dateBuf, acc.c_str(), sizeof(dateBuf));
            }
        break;

        case TTS_LANG_EN:
            {
                std::string acc = _tts_days_en[date->dayOfWeek];
                acc += ',';
                acc += _tts_months_en[date->month - 1];
                acc += ';';
                acc += _tts_number_in_language(date->day, TTS_LANG_EN, TTSNUM_ORDINAL);
                acc += ',';
                strncpy(dateBuf, acc.c_str(), sizeof(dateBuf));
            }
        break;

        case TTS_LANG_JA:
        default:
            snprintf(dateBuf, sizeof(dateBuf), "%s,<NUMK VAL=%i COUNTER=gatsu>;<NUMK VAL=%i COUNTER=nichi>,", _tts_days_ja[date->dayOfWeek], date->month, date->day);
        break;
    }

    return YukkuriUtterance(dateBuf);
}
