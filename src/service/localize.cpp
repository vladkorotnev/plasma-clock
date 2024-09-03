#include <service/localize.h>
#include <service/prefs.h>
#include <device_config.h>
#include <map>

display_language_t active_display_language() {
    return (display_language_t) prefs_get_int(PREFS_KEY_DISP_LANGUAGE);
}

const char * day_letters() {
    static const char en_letters[14] = {
        'S', 0,
        'M', 0,
        'T', 0,
        'W', 0,
        'T', 0,
        'F', 0,
        'S', 0,
    };

    switch(active_display_language()) {
        case DSPL_LANG_EN:
            return en_letters;

        case DSPL_LANG_RU:
            static const char ru_letters[14] = {
                0x82, 0,
                0x8F, 0,
                0x82, 0,
                0x91, 0,
                0x97, 0,
                0x8F, 0,
                0x91, 0,
            };
            return ru_letters;
    }

    return en_letters;
}

static const std::map<const std::string, const char*> english = {
    {"FULL_SETTINGS_NOTICE", "Full settings are only available in the Web UI"},

    {"WEATHER_FMT", "%s. Feels like %.01f\370C. Wind %.01f m/s. Pressure %i hPa."},

#if HAS(BALANCE_BOARD_INTEGRATION)
    {"BB_DSCNCT", "Disconnected"},
    {"BB_CNCT_GUIDE", "\x1A to connect"},

    {"BB_SCN", "Scanning..."},
    {"BB_SYNC_NOW", "Press the SYNC button on the Balance Board now"},
#endif
};

static const std::map<const std::string, const char*> russian = {
    {"Clock", "Часы"},
    {"Timer", "Таймер"},
    {"Stopwatch", "Секундомер"},
    {"Weather", "Погода"},
    {"Alarm", "Будильник"},
    {"Weighing", "Весы"},
    {"Settings", "Параметры"},

    {"Display", "Экран"},
    {"Offsets", "Калибровка"},
    {"Status", "Состояние"},
    {"Save & Restart", "Сохранить и перезагрузить"},

    {"Notice", "Внимание"},
    {"FULL_SETTINGS_NOTICE", "Больше настроек доступно в вебморде через браузер"},

    {"Yes", "Да"},
    {"No", "Нет"},

    {"Uptime", "Время работы"},

    {"Blink dots", "Мигающие точки"},
    {"Tick sound", "Тикание часов"},
    {"Ticking only when screen on", "Тикание только при включённом экране"},
    {"Hourly chime", "Ежечасный сигнал"},
    {"First chime", "Первый за день"},
    {"Other chimes", "Остальные"},
    {"Chime from", "Первый час"},
    {"Chime until", "Последний час"},
    {"Speak hour", "Проговаривать время"},
    {"Speak date on first chime", "Проговаривать дату в первый час"},
    {"Voice speed", "Скорость голоса"},
    {"Set time", "Настроить время"},
    {"Set date", "Настроить дату"},
    {"Use internet time", "Синхронизация времени с интернетом"},
    {"NTP/Timezone", "NTP и часовой пояс"},
    {"Please use the Web UI to configure.", "Настройка доступна только через вебморду"},

    {"Screen Times", "Длительность показа"},
    {"Alarm countdown", "Обратный отсчёт будильника"},
    {"Thermometer", "Термометр"},
    {"Current Weather", "Текущая погода"},
    {"2-day Forecast", "Прогноз на два дня"},
    {"Hourly Precipitation % Graph", "Почасовой график осадков"},
    {"Hourly Pressure Graph", "Почасовой график давления"},

    {"Transition", "Анимация"},
    {"Off", "Отключить"},
    {"Wipe", "Стирание"},
    {"Slide Left", "Сдвиг влево"},
    {"Slide Right", "Сдвиг вправо"},
    {"Slide Up", "Сдвиг вверх"},
    {"Slide Down", "Сдвиг вниз"},
    {"(Randomize)", "(Как повезёт)"},

    {"Scroll Speed", "Скорость бегущей строки"},
    {"Slow", "Медленно"},
    {"Medium", "Средне"},
    {"Fast", "Быстро"},
    {"Sonic", "Совсем быстро"},

    {"FPS counter", "Счётчик кадров в секунду"},
    {"Weather effects", "Эффекты погоды"},

    {"WiFi signal", "Уровень сигнала WiFi"},
    {"Disconnected", "Когда нет связи"},
    {"Display power on", "При включении экрана"},
    {"Always", "Всегда"},

    {"Temperature", "Температура"},
    {"Humidity", "Влажность"},

    {"OS Type", "Операционная система"},
    {"OS Version", "Версия системы"},
    {"WiFi Name", "Имя сети"},
    {"WiFi IP", "Адрес в сети"},
    {"Remote Control Server", "Удалённый доступ"},
    {"Serial MIDI Input", "Эмуляция синтезатора"},

    {"WEATHER_FMT", "%s. Ощущается как %.01f\370C. Ветер %.01f м/с. Давление %i гПа."},
    {"Loading...", "Загрузка..."},
    {"PoP, %", "Осадки, %"},
    {"P, hPa", "Давл., гПа"},

    {"Only Once", "Один раз"},
    {"Enabled", "Включить"},
    {"Repeat on", "Дни недели"},
    {"Time", "Время"},
    {"Smart Alarm", "Лёгкое пробуждение"},
    {"Smart margin", "Запас минут для лёгкого пробуждения"},
    {"Melody", "Мелодия"},
    {"Snooze time", "Поспать ещё, минут"},
    {"Max beeping time, minutes", "Пищать не дольше чем, минут"},

    {"SNOOZE", "СПАТЬ"},
    {"STOP", "ВСТАЮ"},
    {"HOLD", "ДЕРЖИ"},

#if HAS(BALANCE_BOARD_INTEGRATION)
    {"BB_DSCNCT", "Нет связи"},
    {"BB_CNCT_GUIDE", "Нажми \x1A для подключения"},

    {"BB_SCN", "Поиск..."},
    {"BB_SYNC_NOW", "Самое время нажать кнопку SYNC на Balance Board"},
#endif
};

const char * localized_string(const std::string key) {
    const std::map<const std::string, const char*> * lang = nullptr;
    switch(active_display_language()) {
        case DSPL_LANG_EN:
            lang = &english;
            break;
        case DSPL_LANG_RU:
            lang = &russian;
            break;
    }

    if(lang->count(key)) {
        return lang->at(key);
    } else {
        return key.c_str();
    }
}
