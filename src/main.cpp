#include <Arduino.h>
#include <device_config.h>
#include <os_config.h>
#include <display/display.h>
#include <graphics/framebuffer.h>
#include <graphics/screenshooter.h>
#include <graphics/font.h>
#include <sensor/sensors.h>
#include <sound/yukkuri.h>
#include <sound/melodies.h>
#include <network/netmgr.h>
#include <network/otafvu.h>
#include <service/power_management.h>
#include <service/owm/weather.h>
#include <service/time.h>
#include <service/prefs.h>
#include <service/wordnik.h>
#include <service/foo_client.h>
#include <service/alarm.h>
#include <service/disk.h>
#include <network/admin_panel.h>
#include <utils.h>
#include <state.h>
#include <app/app_host.h>
#include <app/idle.h>
#include <app/alarming.h>
#include <app/menu.h>
#include <app/alarm_editor.h>
#include <app/timer_editor.h>
#include <app/stopwatch.h>
#include <app/weighing.h>
#include <app/weather.h>
#include <app/playground.h>
#include <app/bootscreen.h>
#include <app/musicbox.h>
#include <app/new_year.h>
#include <app/httpfvu.h>
#include <app/pixel_cave.h>
#include <sensor/switchbot/meter.h>
#include <views/overlays/fps_counter.h>

static char LOG_TAG[] = "APL_MAIN";

static device_state_t current_state = STATE_BOOT;
static device_state_t _actual_current_state = STATE_BOOT;
static transition_type_t _next_transition = TRANSITION_NONE;
const device_state_t startup_state = STATE_IDLE;
static std::stack<device_state_t> state_stack = {};

static ViewCompositor * desktop;
static ViewMultiplexor * appHost;
static FpsCounter * fpsCounter;

static DisplayFramebuffer * fb;
static Screenshooter * screenshooter;
static FantaManipulator * graph;
static SensorPool * sensors;
static OTAFVUManager * ota;
static Beeper * beepola;
static NewSequencer * seq;
static Yukkuri * yukkuri = nullptr;
static AmbientLightSensor * als = nullptr;

void change_state(device_state_t to, transition_type_t transition) {
#if HAS(OTAFVU)
    if(to == STATE_OTAFVU) {
        current_state = STATE_OTAFVU;
        _actual_current_state = STATE_OTAFVU;
        return; // all other things handled in the FVU process
    }
#endif

    if(to == current_state) return;

    // calling `change_state` from outside of main thread causes a prepare() call on the upcoming view
    // to be out of sequence with step()/render() depending on what the main thread was up to at the time.
    // So just keep the requested change in memory and do it when main thread gets around to it.
    // might we need a queue here?
    _next_transition = transition;
    current_state = to;

    if(to == STATE_IDLE) {
        state_stack.empty();
    }
}

void push_state(device_state_t next, transition_type_t transition) {
    if(next == current_state) return;

    state_stack.push(current_state);
    change_state(next, transition);
}

void pop_state(device_state_t expected, transition_type_t transition) {
    if(!state_stack.empty()) {
        device_state_t old = state_stack.top();
        state_stack.pop();
        change_state(old, transition);
    }
}

device_state_t get_state() {
    return _actual_current_state;
}

void bringup_sound() {
    beepola = new Beeper();
    seq = new NewSequencer();

#if HAS(AQUESTALK)
    String license = prefs_get_string(PREFS_KEY_VOICE_LICENSE, AQUESTALK_LICENSE_KEY);
    yukkuri = new Yukkuri(license.c_str());
    WaveOut::set_output_callback(pisosWAVE_CHANNEL_YUKKURI, yukkuri->get_callback());
#endif

    WaveOut::set_output_callback(pisosWAVE_CHANNEL_BEEPER, beepola->get_callback());
    WaveOut::set_output_callback(pisosWAVE_CHANNEL_SEQUENCER, seq->get_callback());

    ESP_LOGI(LOG_TAG, "Sound subsystem ready");
}

void bringup_light_sensor() {
#if HAS(LIGHT_SENSOR)
    als = new AmbientLightSensor(HWCONF_LIGHTSENSE_GPIO);
    sensors->add(SENSOR_ID_AMBIENT_LIGHT, als, pdMS_TO_TICKS(250));
    ESP_LOGI(LOG_TAG, "Light sensor ready");
#endif
}

void bringup_motion_sensor() {
#if HAS(MOTION_SENSOR)
    sensors->add(SENSOR_ID_MOTION, new MotionSensor(HWCONF_MOTION_GPIO), pdMS_TO_TICKS(250));
    ESP_LOGI(LOG_TAG, "Motion sensor ready");
#endif
}

void bringup_temp_sensor() {
#if HAS(TEMP_SENSOR)
    Wire.begin(HWCONF_I2C_SDA_GPIO, HWCONF_I2C_SCL_GPIO);
    AM2322* tempSens = new AM2322(&Wire);

    if(!sensors->add(SENSOR_ID_AMBIENT_TEMPERATURE, new Am2322TemperatureSensor(tempSens), pdMS_TO_TICKS(5000))) {
        beepola->beep(CHANNEL_SYSTEM, 500, 125);
        return;
    } 
    ESP_LOGI(LOG_TAG, "Temperature sensor ready");

    delay(1000);
    if(!sensors->add(SENSOR_ID_AMBIENT_HUMIDITY, new Am2322HumiditySensor(tempSens), pdMS_TO_TICKS(5000))) {
        beepola->beep(CHANNEL_SYSTEM, 500, 125);
        return;
    }
    ESP_LOGI(LOG_TAG, "Humidity sensor ready");
#endif
}

void bringup_switchbot_sensor() {
#if HAS(SWITCHBOT_METER_INTEGRATION)
    if(prefs_get_bool(PREFS_KEY_SWITCHBOT_METER_ENABLE)) {
        String woMeterMac = prefs_get_string(PREFS_KEY_SWITCHBOT_METER_MAC);
        size_t tmpLen = woMeterMac.length();
        if(tmpLen > 0) {
            SwitchbotMeterApi *wometer = new SwitchbotMeterApi(woMeterMac.c_str());

            SwitchbotMeterHumidity *remoteHum = new SwitchbotMeterHumidity(wometer);
            SwitchbotMeterTemperature *remoteTemp = new SwitchbotMeterTemperature(wometer);

            if(!sensors->add(SENSOR_ID_SWITCHBOT_INDOOR_HUMIDITY, remoteHum, pdMS_TO_TICKS(30000))) {
                ESP_LOGE(LOG_TAG, "WoH err");
            } else {
                if(prefs_get_bool(PREFS_KEY_SWITCHBOT_EMULATES_LOCAL) && !sensors->exists(SENSOR_ID_AMBIENT_HUMIDITY)) {
                    sensors->short_circuit(SENSOR_ID_AMBIENT_HUMIDITY, SENSOR_ID_SWITCHBOT_INDOOR_HUMIDITY);
                }
            }

            if(!sensors->add(SENSOR_ID_SWITCHBOT_INDOOR_TEMPERATURE, remoteTemp, pdMS_TO_TICKS(30000))) {
                ESP_LOGE(LOG_TAG, "WoT err");
            } else {
                if(prefs_get_bool(PREFS_KEY_SWITCHBOT_EMULATES_LOCAL) && !sensors->exists(SENSOR_ID_AMBIENT_TEMPERATURE)) {
                    sensors->short_circuit(SENSOR_ID_AMBIENT_TEMPERATURE, SENSOR_ID_SWITCHBOT_INDOOR_TEMPERATURE);
                }
            }

            ESP_LOGI(LOG_TAG, "WoSensor ready");
        }
    }
#endif
}

void bringup_hid() {
#if HAS(TOUCH_PLANE)
    if(touchplane_start() != ESP_OK) {
        ESP_LOGE(LOG_TAG, "TP init err");
        beepola->beep(CHANNEL_SYSTEM, 500, 125);
    }
    ESP_LOGI(LOG_TAG, "Touchpad ready");
#endif
#if HAS(KEYPAD)
    keypad_start();
    ESP_LOGI(LOG_TAG, "Keypad ready");
#endif
#if HAS(IR_RECEIVER)
    infrared_start();
    ESP_LOGI(LOG_TAG, "Infrared ready");
#endif
    if(prefs_get_bool(PREFS_KEY_BUTTON_BEEP)) {
        hid_set_key_beeper(beepola);
    }
    sensors->add(VIRTSENSOR_ID_HID_STARTLED, hid_get_state_sensor(), pdMS_TO_TICKS(100));
}

static TaskHandle_t bootTaskHandle = NULL;
void boot_task(void*) {
    ESP_LOGI(LOG_TAG, PRODUCT_NAME " v" PRODUCT_VERSION " is in da house now!!");

    NetworkManager::startup();
    ESP_LOGI(LOG_TAG, "NetMgr started");
    while(!NetworkManager::is_up()) {
        delay(1000);
        ESP_LOGI(LOG_TAG, "Waiting for NetMgr...");
    }

    screenshooter = new Screenshooter(fb->manipulate());
    if(prefs_get_bool(PREFS_KEY_REMOTE_SERVER)) {
        screenshooter->start_server(3939);
        ESP_LOGI(LOG_TAG, "RC Server ready");
    }

    ota = new OTAFVUManager(seq);
    ESP_LOGI(LOG_TAG, "OTAFVUMgr ready");

    sensors = new SensorPool();
    ESP_LOGI(LOG_TAG, "Creating sensor pool");

    sensors->add(VIRTSENSOR_ID_WIRELESS_RSSI, new RssiSensor(), pdMS_TO_TICKS(500));
    bringup_light_sensor();
    bringup_motion_sensor();
    bringup_temp_sensor();
    bringup_switchbot_sensor();
    bringup_hid();

    ESP_LOGI(LOG_TAG, "Loading music database");
    load_melodies_from_disk();

    ESP_LOGI(LOG_TAG, "Starting services...");
    timekeeping_begin();
    weather_start();
    wotd_start();
    foo_client_begin();
    power_mgmt_start(sensors, &display_driver, beepola);
    admin_panel_prepare(sensors, beepola);

    ESP_LOGI(LOG_TAG, "Constructing desktop");
    appHost->add_view(new AppShimIdle(sensors, beepola, seq, yukkuri), STATE_IDLE);
    appHost->add_view(new AppShimAlarming(seq), STATE_ALARMING);
    appHost->add_view(new AppShimMenu(beepola, seq, yukkuri, als), STATE_MENU);
    appHost->add_view(new AppShimAlarmEditor(beepola, seq), STATE_ALARM_EDITOR);
    appHost->add_view(new AppShimTimerEditor(beepola, seq), STATE_TIMER_EDITOR);
    appHost->add_view(new AppShimStopwatch(beepola), STATE_STOPWATCH);
    appHost->add_view(new AppShimWeather(), STATE_WEATHER);
    appHost->add_view(new AppShimMusicbox(seq), STATE_MUSICBOX);
#if HAS(BALANCE_BOARD_INTEGRATION)
    appHost->add_view(new AppShimWeighing(sensors), STATE_WEIGHING);
#endif
#if HAS(PLAYGROUND)
    appHost->add_view(new AppShimPlayground(), STATE_PLAYGROUND);
#endif
    appHost->add_view(new NewYearAppShim(beepola, seq, yukkuri), STATE_NEW_YEAR);
#if HAS(HTTPFVU)
    appHost->add_view(new HttpFvuApp(seq), STATE_HTTPFVU);
#endif
#if HAS(PIXEL_CAVE)
    appHost->add_view(new AppPixelCave(seq), STATE_PIXEL_CAVE);
#endif

    ESP_LOGI(LOG_TAG, "Finishing up");
    change_state(startup_state, TRANSITION_WIPE);
    alarm_init(sensors);

    ESP_LOGI(LOG_TAG, "Shut up and explode!");
    bootTaskHandle = NULL;
    vTaskDelete(NULL);
}

void setup() {
    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 2);
    // Set up serial for logs
    Serial.begin(115200);
    while(!Serial);
#ifdef BOARD_HAS_PSRAM
    heap_caps_malloc_extmem_enable(16);
#endif
    ESP_LOGI(LOG_TAG, "Hello!");

    // The I2S driver messes up display pinmux, so it must initialize first
    WaveOut::init_I2S(HWCONF_BEEPER_GPIO);

    display_driver.initialize();
    display_driver.reset();

    display_driver.set_power(true);
    delay(125);
    display_driver.set_show(true);
#if HAS(VARYING_BRIGHTNESS)
    display_driver.set_bright(true);
#endif

    bringup_sound();
    if (!mount_disk(true)) {
        ESP_LOGE(LOG_TAG, "An Error has occurred while mounting file system");
        seq->play_sequence(&tulula_fvu);
    } else {
        ESP_LOGI(LOG_TAG, "File system mounted");
        seq->play_sequence(&pc98_pipo);
    }

    load_fonts();

    fb = new DisplayFramebuffer(&display_driver);

    desktop = new ViewCompositor();
    appHost = new AppHost();
    desktop->add_layer(appHost);
    desktop->add_layer(new FpsCounter(fb));

    appHost->add_view(new Bootscreen(), STATE_BOOT);
    appHost->add_view(new RebootScreen(), STATE_RESTART);

    graph = fb->manipulate();
    fb->clear();

    if(xTaskCreate(
        boot_task,
        "BOOT",
        8192,
        nullptr,
        configMAX_PRIORITIES - 1,
        &bootTaskHandle
    ) == pdFALSE) {
        ESP_LOGE(LOG_TAG, "Boot task creation failure");
        ESP.restart();
    }
    ESP_LOGI(LOG_TAG, "setup end.");
}

static TickType_t memory_last_print = 0;
static void print_memory() {
    TickType_t now = xTaskGetTickCount();
    if(now - memory_last_print > pdMS_TO_TICKS(30000)) {
        ESP_LOGI(LOG_TAG, "HEAP: %d free of %d (%d minimum)", ESP.getFreeHeap(), ESP.getHeapSize(), ESP.getMinFreeHeap());
#ifdef BOARD_HAS_PSRAM
        ESP_LOGI(LOG_TAG, "PSRAM: %d free of %d (%d minimum)", ESP.getFreePsram(), ESP.getPsramSize(), ESP.getMinFreePsram());
#endif
        tk_time_of_day_t uptime = get_uptime();
        ESP_LOGI(LOG_TAG, "Uptime: %02dh%02dm%02ds%03dms", uptime.hour, uptime.minute, uptime.second, uptime.millisecond);
        memory_last_print = now;
    }
}

bool bumTheDog = false;
void loop() {
    if(!bumTheDog) {
        disableLoopWDT();
        disableCore0WDT();
        disableCore1WDT();
        bumTheDog = true;
    }
    
#if HAS(OTAFVU)
    if(_actual_current_state != STATE_OTAFVU) { // OTAFVU basically locks everything down until reboot
#endif
        fb->wait_next_frame();
        if(graph->lock()) {
            desktop->render(graph);
            graph->unlock();
        }
        desktop->step();
        
        // thread safe state change kind of thing
        if(_actual_current_state != current_state) {
            _actual_current_state = current_state;
            appHost->switch_to(_actual_current_state, _next_transition);
        }
        print_memory();
#if HAS(OTAFVU)
    } else {
        if(_actual_current_state != current_state && current_state == STATE_RESTART) {
            _actual_current_state = current_state;
            appHost->switch_to(_actual_current_state, _next_transition);
        }
    }
#endif
}
