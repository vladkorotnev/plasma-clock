#include <service/balance_board.h>
#include <service/prefs.h>
#if HAS(BALANCE_BOARD_INTEGRATION)
#include <Wiimote.h>

static char LOG_TAG[] = "BBRD";

EXT_RAM_ATTR Wiimote wiimote;
static balance_board_state_t sts;
static int kilos = 0;
static int offset = 0;
static bool initialized = false;
static FauxSensor *kiloSensor;
static TimerSensor *startledSensor;
static TaskHandle_t hTask;
static uint16_t hRemote = 0;
static TickType_t lastUpdate = 0;
static TickType_t lastBlink = 0;
static bool lastLed = false;
static bool forceLed = false;
static bool button = false;
// Low-pass filter parameters
static const float alpha = 0.1f; // Smoothing factor (0.0 - 1.0)
static int filteredKilos = 0;    // Filtered weight value
static int avgKilos = 0;

void balance_board_zero()
{
  offset = kilos;
  prefs_set_int(PREFS_KEY_BALANCE_BOARD_OFFSET, offset);
}

void wiimote_callback(wiimote_event_type_t event_type, uint16_t handle, uint8_t *data, size_t len)
{
  if (event_type == WIIMOTE_EVENT_DATA)
  {
    if (len > 11 && data[1] == 0x34)
    {
      TickType_t now = xTaskGetTickCount();

      float weight[4];
      wiimote.get_balance_weight(data, weight);
      float measurement = weight[BALANCE_POSITION_TOP_LEFT] + weight[BALANCE_POSITION_TOP_RIGHT] + weight[BALANCE_POSITION_BOTTOM_LEFT] + weight[BALANCE_POSITION_BOTTOM_RIGHT];
      int current = (int)(trunc(measurement * 100.0));

      // Apply low-pass filter
      filteredKilos = (int)(alpha * current + (1.0f - alpha) * filteredKilos);

      avgKilos += filteredKilos;
      avgKilos /= 2;

      if (now - lastUpdate >= pdMS_TO_TICKS(100))
      {
        if (abs(avgKilos - kilos) >= 2000)
        {
          startledSensor->trigger();
        }
        kilos = avgKilos;
        kiloSensor->value = (kilos - offset);

        ESP_LOGV(LOG_TAG, "Balance board value: %f kg", kilos);
        lastUpdate = now;
      }

      button = (data[3] & 0x08) != 0;
      if (button)
      {
        startledSensor->trigger();
      }

      if(!forceLed) {
        if(now - lastBlink >= pdMS_TO_TICKS((lastLed ? 125 : 120000))) {
          lastLed = !lastLed;
          wiimote.set_led(handle, lastLed ? 1 : 0);
          lastBlink = now;
        }
      }
    }
  }
  else if (event_type == WIIMOTE_EVENT_INITIALIZE)
  {
    ESP_LOGI(LOG_TAG, "WIIMOTE_EVENT_INITIALIZE\n");
    if (sts != BB_CONNECTED)
      balance_board_scan(true);
  }
  else if (event_type == WIIMOTE_EVENT_SCAN_START)
  {
    ESP_LOGI(LOG_TAG, "WIIMOTE_EVENT_SCAN_START");
    sts = BB_SCANNING;
  }
  else if (event_type == WIIMOTE_EVENT_SCAN_STOP)
  {
    ESP_LOGI(LOG_TAG, "WIIMOTE_EVENT_SCAN_STOP");
    if (sts != BB_CONNECTED)
      sts = BB_IDLE;
  }
  else if (event_type == WIIMOTE_EVENT_CONNECT)
  {
    ESP_LOGI(LOG_TAG, "WIIMOTE_EVENT_CONNECT");
    sts = BB_CONNECTED;
    hRemote = handle;
  }
  else if (event_type == WIIMOTE_EVENT_DISCONNECT)
  {
    ESP_LOGI(LOG_TAG, "  event_type=WIIMOTE_EVENT_DISCONNECT\n");
    sts = BB_IDLE;
  }
}

void bb_task(void *)
{
  while (1)
  {
    wiimote.handle();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void balance_board_start(SensorPool *sensors)
{
  if (initialized)
    return;
  wiimote.init(wiimote_callback);
  sts = BB_IDLE;
  offset = prefs_get_int(PREFS_KEY_BALANCE_BOARD_OFFSET);
  kiloSensor = new FauxSensor(0);
  startledSensor = new TimerSensor(pdMS_TO_TICKS(500));
  sensors->add(SENSOR_ID_BALANCE_BOARD_MILLIKILOS, kiloSensor, pdMS_TO_TICKS(250));
  sensors->add(SENSOR_ID_BALANCE_BOARD_STARTLED, startledSensor, pdMS_TO_TICKS(250));
  if (!xTaskCreate(
          bb_task,
          LOG_TAG,
          8192,
          nullptr,
          2,
          &hTask))
  {
    ESP_LOGE(LOG_TAG, "Task creation failed");
  }
  initialized = true;
}

void balance_board_scan(bool state)
{
  wiimote.scan(state);
}

balance_board_state_t balance_board_state()
{
  return sts;
}

void balance_board_led(bool ledSts) {
  if(sts != BB_CONNECTED) return;

  forceLed = ledSts;
  wiimote.set_led(hRemote, ledSts ? 1 : 0);
}

#endif
