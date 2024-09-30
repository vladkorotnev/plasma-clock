#include <sound/sequencer.h>
#include <os_config.h>
#include <service/prefs.h>
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
            ESP_LOGV(LOG_TAG, "MIDI recv");
            seq->midi_task();
        }
        taskYIELD();
    }
}

void NewSequencer::midi_task() {
    switch(midiIo.getType()) {
        case midi::NoteOn:
            { 
                int ch = midiIo.getChannel() - 1;
                if(ch < 0 || ch >= NewSequencer::CHANNELS) {
                    ESP_LOGV(LOG_TAG, "Invalid channel %i", ch);
                    return;
                }
                int noteNo = midiIo.getData1();
                lastNote[ch] = noteNo;
                int velo = midiIo.getData2();
                ESP_LOGV(LOG_TAG, "Midi Note On ch=%i, noteNo=%i, velo=%i, freq=%i", ch, noteNo, velo, sNotePitches[noteNo]/2);
                voices[ch]->set_parameter(ToneGenerator::Parameter::PARAMETER_FREQUENCY, velo > 0 ? sNotePitches[noteNo]/2 : 0);
            }
            break;

        case midi::NoteOff:
            {
                int ch = midiIo.getChannel() - 1;
                if(ch < 0 || ch >= NewSequencer::CHANNELS) {
                    ESP_LOGV(LOG_TAG, "Invalid channel %i", ch);
                    return;
                }
                if(lastNote[ch] == midiIo.getData1()) {
                    ESP_LOGV(LOG_TAG, "Midi Note Off ch=%i", ch);
                    voices[ch]->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, false);
                }
            }
            break;

        case midi::ControlChange:
            {
                int ch = midiIo.getChannel() - 1;
                if(ch < 0 || ch >= NewSequencer::CHANNELS) {
                    ESP_LOGV(LOG_TAG, "Invalid channel %i", ch);
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

static const uint8_t kick_rle_data[] = {0, 7, 4, 2, 1, 24, 23, 30, 33, 26, 38, 38, 30, 41, 41, 52, 62, 50, 58, 64, 61, 70, 99, 92, 80, 119, 102, 119, 119, 142, 146, 117, 160, 119, 154, 5};
const rle_sample_t kick_sample = { .sample_rate = 8000, .root_frequency = 524 /* C5 */, .length = 36, .mode = MIX_MODE_XOR, .rle_data = kick_rle_data };

NewSequencer::NewSequencer() {
    num_rows = 0;
    rows = nullptr;
    // Ch 0, 1, 2, 3: tone
    for(int i = 0; i < TONE_CHANNELS; i++) voices[i] = new SquareGenerator();
    voices[4] = new NoiseGenerator(); // Ch4: Noise
    voices[5] = new Sampler(&kick_sample); // Ch5: RLE PWM
    wait_end_group = xEventGroupCreate();

#if HAS(SERIAL_MIDI)
    if(prefs_get_bool(PREFS_KEY_SERIAL_MIDI)) {
        xTaskCreate(
            serMidiTask,
            "MIDI",
            2048,
            this,
            pisosTASK_PRIORITY_SERIAL_MIDI,
            &hMidiTask
        );
    }
#endif
}

NewSequencer::~NewSequencer() {
    for(int i = 0; i < CHANNELS; i++) delete voices[i];
}

bool NewSequencer::is_sequencing() {
    return is_running;
}

void NewSequencer::stop_sequence() {
    is_running = false;
    for(int i = 0; i < CHANNELS; i++) voices[i]->set_parameter(ToneGenerator::PARAMETER_ACTIVE, false);
    voices[5]->set_parameter(ToneGenerator::PARAMETER_SAMPLE_ADDR, (int) &kick_sample);
    sequence->unload();
    num_rows = 0;
    rows = nullptr;
    xEventGroupSetBits(wait_end_group, BIT_END_PLAY);
}

void NewSequencer::wait_end_play() {
    xEventGroupWaitBits(wait_end_group, BIT_END_PLAY, pdFALSE, pdTRUE, portMAX_DELAY);
}

void NewSequencer::play_sequence(MelodySequence * s, sequence_playback_flags_t f, int repeat) {
    if(!s->load()) {
        ESP_LOGE(LOG_TAG, "Failed to load sequence");
        return;
    }
    repetitions = repeat;
    sequence = s;
    rows = s->get_array();
    num_rows = s->get_num_rows();
    flags = f;
    pointer = 0;
    loop_point = 0;
    for(int i = 0; i < CHANNELS; i++) {
        voices[i]->set_parameter(ToneGenerator::PARAMETER_FREQUENCY, 0);
        voices[i]->set_parameter(ToneGenerator::PARAMETER_DUTY, 0);
    }
    voices[5]->set_parameter(ToneGenerator::PARAMETER_SAMPLE_ADDR, (int) &kick_sample);
    if((f & SEQUENCER_PLAY_HOOK_ONLY) != 0) {
        find_hook();
        pointer = loop_point;
    }
    remaining_delay_samples = 0;
    xEventGroupClearBits(wait_end_group, BIT_END_PLAY);
    process_steps_until_delay();
    is_running = true;
}

bool NewSequencer::end_of_song() {
    // End of Song condition
    if((flags & SEQUENCER_REPEAT_INDEFINITELY) != 0 || repetitions > 0) {
        if(repetitions > 0) repetitions--;
        pointer = loop_point;
        process_steps_until_delay();
        return true;
    } else if(repetitions == 0) {
        stop_sequence();
        return true;
    }

    return false;
}

bool NewSequencer::process_step(const melody_item_t * cur_line) {
    switch(cur_line->command) {
        case FREQ_SET:
            voices[cur_line->channel]->set_parameter(ToneGenerator::PARAMETER_FREQUENCY, cur_line->argument);
            break;
        case DUTY_SET:
            voices[cur_line->channel]->set_parameter(ToneGenerator::PARAMETER_DUTY, cur_line->argument);
            break;
        case LOOP_POINT_SET:
            if(cur_line->argument == LOOP_POINT_TYPE_HOOK_END && (flags & SEQUENCER_PLAY_HOOK_ONLY) != 0) {
                if(end_of_song()) return true;
            }
            else if(cur_line->argument == LOOP_POINT_TYPE_LOOP) {
                loop_point = pointer + 1;
            }
            break;
        case DELAY:
            remaining_delay_samples = cur_line->argument * WaveOut::BAUD_RATE / 1000;
            break;
        case SAMPLE_LOAD:
            voices[cur_line->channel]->set_parameter(ToneGenerator::PARAMETER_SAMPLE_ADDR, cur_line->argument);
            break;
        default:
            break;
    }

    return false;
}

void NewSequencer::find_hook() {
    while(pointer < num_rows) {
        const melody_item_t * cur_line = &rows[pointer];
        if(cur_line->command == LOOP_POINT_SET) {
            if(cur_line->argument == LOOP_POINT_TYPE_HOOK_START) {
                loop_point = pointer + 1;
                ESP_LOGI(LOG_TAG, "Hook found, starts at %i", loop_point);
                break;
            } else {
                pointer++;
            }
        } else if(cur_line->command == DELAY || cur_line->command == LOOP_POINT_SET || cur_line->command == FREQ_SET) {
            pointer++;
        } else {
            process_step(cur_line);
            pointer++;
        }
    }
}

void NewSequencer::process_steps_until_delay() {
    if(num_rows == 0) return;
    
    if(pointer >= num_rows) {
        if(end_of_song()) return;
    }

    const melody_item_t * cur_line = &rows[pointer];
    if(process_step(cur_line)) return;

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
