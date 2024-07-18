#include <sound/sequencer.h>

static char LOG_TAG[] = "MELODY";

#define EVT_BIT_ENDED_PLAYING (1 << 0)

extern "C" void MelodyTaskFunction( void * pvParameter );

void MelodyTaskFunction( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "Running task");
    BeepSequencer * that = static_cast<BeepSequencer*> ( pvParameter );
    that->task();
}


BeepSequencer::BeepSequencer(Beeper * b) {
    beeper = b;
    is_running = false;
    pointer = 0;
    hTask = NULL;
    waitEndGroup = xEventGroupCreate();
}

BeepSequencer::~BeepSequencer() {
   end_task(); 
}

void BeepSequencer::play_sequence(const melody_sequence_t melody, beeper_channel_t ch, int repeat) {
    pointer = 0;
    timbre = DUTY_SQUARE;
    current_sequence = melody;
    current_channel = ch;
    repetitions = repeat;
    is_running = true;
    
    start_task();
}

void BeepSequencer::start_task() {
    if(hTask != NULL) return;

    if(xTaskCreate(
        MelodyTaskFunction,
        "MELODY",
        4096,
        this,
        configMAX_PRIORITIES - 1,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

void BeepSequencer::end_task() {
    if(hTask != NULL) {
        vTaskDelete(hTask);
        hTask = NULL;
    }

    xEventGroupSetBits(waitEndGroup, EVT_BIT_ENDED_PLAYING);
}

void BeepSequencer::stop_sequence() {
    if(!is_running) return;
    is_running = false;
    end_task();
    beeper->stop_tone(current_channel);
}

void BeepSequencer::task() {
    while(is_running) {
        melody_item_t cur = current_sequence.array[pointer];
        if(cur.length == SEQ_LEN_FLAG_TIMBRE_SET) {
            timbre = cur.frequency;
        } else {
            if(cur.frequency == 0) {
                beeper->stop_tone(current_channel);
            } else {
                beeper->start_tone(current_channel, cur.frequency, timbre);
            }
            vTaskDelay(cur.length);
        }

        pointer ++;

        if(pointer >= current_sequence.count) {
            if(repetitions > 0) {
                repetitions--;
                pointer = 0;
            } else if(repetitions < 0) {
                pointer = 0;
            } else {
                beeper->stop_tone(current_channel);
                is_running = false;
                hTask = NULL;
                xEventGroupSetBits(waitEndGroup, EVT_BIT_ENDED_PLAYING);
                vTaskDelete(NULL);
            }
        }
    }
}

void BeepSequencer::wait_end_play() {
    xEventGroupClearBits(waitEndGroup, EVT_BIT_ENDED_PLAYING);
    xEventGroupWaitBits(waitEndGroup, EVT_BIT_ENDED_PLAYING, false, true, portMAX_DELAY);
}