#include <sound/sequencer.h>
#include <os_config.h>
#include <algorithm>

static char LOG_TAG[] = "MELODY";

#define BIT_END_PLAY 0b00000001

NewSequencer::NewSequencer() {
    // Ch 0, 1, 2, 3: tone
    for(int i = 0; i < TONE_CHANNELS; i++) voices[i] = new SquareGenerator();
    voices[4] = new NoiseGenerator(); // Ch4: Noise
    voices[5] = new Sampler(); // Ch5: RLE PWM
    wait_end_group = xEventGroupCreate();
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
    if(!is_running) return 0;
    if(remaining_delay_samples == 0) process_steps_until_delay();

    size_t generated = 0;
    uint32_t want_samples = std::min(length * 8, (size_t) remaining_delay_samples);
    for(int i = 0; i < CHANNELS; i++) {
        size_t cur = voices[i]->generate_samples(buffer, length, want_samples);
        if(cur > generated) generated = cur;
    }

    remaining_delay_samples -= want_samples;

    return generated;
}