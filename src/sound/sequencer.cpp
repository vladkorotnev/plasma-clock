#include <sound/sequencer.h>
#include <os_config.h>
#include <algorithm>

static char LOG_TAG[] = "MELODY";

#define BIT_END_PLAY 0b00000001

#if HAS(SERIAL_MIDI)
#include <MIDI.h>

static const uint16_t sNotePitches[] = {
    31, 33, 35, 37, 39, 41, 44, 46,
    49, 52, 55, 58, 62, 65, 69, 73,
    78, 82, 87, 93, 98, 104, 110, 117,
    123, 131, 139, 147, 156, 165, 175, 185,
    196, 208, 220, 233, 247, 262, 277, 294,
    311, 330, 349, 370, 392, 415, 440, 466,
    494, 523, 554, 587, 622, 659, 698, 740,
    784, 831, 880, 932, 988, 1047, 1109, 1175,
    1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865,
    1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960,
    3136, 3322, 3520, 3729, 3951, 4186, 4435, 4699, 4978,
};

struct MySettings : public midi::DefaultSettings
{
   static const unsigned SysExMaxSize = 1024; // Accept SysEx messages up to 1024 bytes long.
   static const unsigned BaudRate = 115200;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, midiIo, MySettings);

static void serMidiTask(void * pvParameters) {
    NewSequencer * seq = static_cast<NewSequencer*>(pvParameters);
    midiIo.begin();
    midiIo.setInputChannel(MIDI_CHANNEL_OMNI);
    midiIo.turnThruOff();
    while(1) {
        if(midiIo.read()) {
            ESP_LOGI(LOG_TAG, "MIDI recv");
            seq->midi_task();
        }
    }
}

void NewSequencer::midi_task() {
    switch(midiIo.getType()) {
        case midi::NoteOn:
            { 
                int ch = midiIo.getChannel() - 1;
                if(ch < 0 || ch >= NewSequencer::CHANNELS) {
                    ESP_LOGI(LOG_TAG, "Invalid channel %i", ch);
                    return;
                }
                int noteNo = midiIo.getData1();
                int velo = midiIo.getData2();
                ESP_LOGV(LOG_TAG, "Midi Note On ch=%i, noteNo=%i, velo=%i, freq=%i", ch, noteNo, velo, sNotePitches[noteNo]/2);
                voices[ch]->set_parameter(ToneGenerator::Parameter::PARAMETER_FREQUENCY, velo > 0 ? sNotePitches[noteNo]/2 : 0);
            }
            break;

        case midi::NoteOff:
            {
                int ch = midiIo.getChannel() - 1;
                if(ch < 0 || ch >= NewSequencer::CHANNELS) {
                    ESP_LOGI(LOG_TAG, "Invalid channel %i", ch);
                    return;
                }
                ESP_LOGV(LOG_TAG, "Midi Note Off ch=%i", ch);
                voices[ch]->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, false);
            }
            break;

        case midi::ControlChange:
            {
                int ch = midiIo.getChannel() - 1;
                if(ch < 0 || ch >= NewSequencer::CHANNELS) {
                    ESP_LOGI(LOG_TAG, "Invalid channel %i", ch);
                    return;
                }
                int controlNo = midiIo.getData1();
                int controlVal = midiIo.getData2();
                switch(controlNo) {
                    case 2:
                        voices[ch]->set_parameter(ToneGenerator::Parameter::PARAMETER_DUTY, controlVal);
                        break;
                    default:
                        ESP_LOGV(LOG_TAG, "Invalid control %i", controlNo);
                        break;
                }
            }
            break;
    }
}

#endif

NewSequencer::NewSequencer() {
    // Ch 0, 1, 2, 3: tone
    for(int i = 0; i < TONE_CHANNELS; i++) voices[i] = new SquareGenerator();
    voices[4] = new NoiseGenerator(); // Ch4: Noise
    voices[5] = new Sampler(); // Ch5: RLE PWM
    wait_end_group = xEventGroupCreate();
    xTaskCreate(
        serMidiTask,
        "MIDI",
        2048,
        this,
        pisosTASK_PRIORITY_SERIAL_MIDI,
        &hMidiTask
    );
}

NewSequencer::~NewSequencer() {
    for(int i = 0; i < CHANNELS; i++) delete voices[i];
}

void NewSequencer::stop_sequence() {
    is_running = false;
    for(int i = 0; i < CHANNELS; i++) voices[i]->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, false);
    xEventGroupSetBits(wait_end_group, BIT_END_PLAY);
}

void NewSequencer::wait_end_play() {
    xEventGroupWaitBits(wait_end_group, BIT_END_PLAY, pdFALSE, pdTRUE, portMAX_DELAY);
}

void NewSequencer::play_sequence(const melody_sequence_t * s, int repeat) {
    repetitions = repeat;
    sequence = s;
    pointer = 0;
    loop_point = 0;
    is_running = true;
    remaining_delay_samples = 0;
    for(int i = 0; i < CHANNELS; i++) voices[i]->set_parameter(ToneGenerator::Parameter::PARAMETER_DUTY, 0);
    xEventGroupClearBits(wait_end_group, BIT_END_PLAY);
    process_steps_until_delay();
}

void NewSequencer::process_steps_until_delay() {
    if(!is_running) return;
    if(pointer >= sequence->count) {
        // End of Song condition
        if(repetitions == -1 || repetitions > 0) {
            if(repetitions > 0) repetitions--;
            pointer = loop_point;
            process_steps_until_delay();
            return;
        } else if(repetitions == 0) {
            stop_sequence();
            return;
        }
    }

    const melody_item_t * cur_line = &sequence->array[pointer];
    switch(cur_line->command) {
        case FREQ_SET:
            voices[cur_line->channel]->set_parameter(ToneGenerator::Parameter::PARAMETER_FREQUENCY, cur_line->argument1);
            // if(cur_line->argument1 > 0) for(int i = 0; i < TONE_CHANNELS; i++) voices[i]->reset_phase(); // retrig for unison
            break;
        case DUTY_SET:
            voices[cur_line->channel]->set_parameter(ToneGenerator::Parameter::PARAMETER_DUTY, cur_line->argument1);
            break;
        case LOOP_POINT_SET:
            loop_point = pointer + 1;
            break;
        case DELAY:
            remaining_delay_samples = cur_line->argument1 * WaveOut::BAUD_RATE / 1000;
            break;
        case SAMPLE_LOAD:
            voices[cur_line->channel]->set_parameter(ToneGenerator::Parameter::PARAMETER_SAMPLE_ADDR, cur_line->argument1);
            break;
        default:
            break;
    }

    pointer++;

    if(cur_line->command != DELAY) {
        process_steps_until_delay();
    }
}

size_t NewSequencer::fill_buffer(void* buffer, size_t length) {
#if !HAS(SERIAL_MIDI)
    if(!is_running) return 0;
#endif
    if(is_running && remaining_delay_samples == 0) process_steps_until_delay();

    size_t generated = 0;
    uint32_t want_samples = std::min(length * 8, (size_t) remaining_delay_samples);
    for(int i = 0; i < CHANNELS; i++) {
        size_t cur = voices[i]->generate_samples(buffer, length, want_samples);
        if(cur > generated) generated = cur;
    }

    remaining_delay_samples -= want_samples;

    return generated;
}
