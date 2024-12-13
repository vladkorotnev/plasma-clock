#include <app/musicbox.h>
#include <service/prefs.h>
#include <service/localize.h>
#include <device_config.h>
#include <views/framework.h>
#include <state.h>
#include <algorithm>

AppShimMusicbox::AppShimMusicbox(NewSequencer * s): 
    rain { new RainOverlay(DisplayFramebuffer::width, DisplayFramebuffer::height) },
    thunder { new ThunderOverlay(DisplayFramebuffer::width, DisplayFramebuffer::height) },
    fireworks { new FireworksOverlay(nullptr) },
    seq { s },
    WeatherChartCommon()
{
    add_subrenderable(rain);
    add_subrenderable(thunder);
    add_subrenderable(fireworks);

    rain->set_intensity(0);
    rain->set_gravity(2, true);
    rain->set_windspeed(0, false);

    thunder->set_active(false);

    fireworks->set_active(false);
    fireworks->intense = true;

    show_minimum = false;
    show_maximum = false;
    
    filled = true;
    autoscale = false;
    minimum = 0;
    maximum = 20;
    hint_lbl->stopped = false;
    hint_lbl->set_y_position(4);
    hint_lbl->scroll_only_if_not_fit = false;
    hint_lbl->start_at_visible = false;
    hint_lbl->reappear_delay = 150;
    trackNo = maxTrackNo;
#if HAS(SERIAL_MIDI)
    if(prefs_get_bool(PREFS_KEY_SERIAL_MIDI)) {
        maxTrackNo ++;
    }
#endif
}

void AppShimMusicbox::render(FantaManipulator* fb) {
    if(points.size() < fb->get_width()) {
        for(int i = 0; i < fb->get_width(); i++) {
            points.push_back( { .annotation = -1, .value = 0 });
        }
    }
    WeatherChartCommon::render(fb);
}

void AppShimMusicbox::prepare() {
    WeatherChartCommon::prepare();
    load_and_play();
}

void AppShimMusicbox::step() {
    int thunder_decision = 0;
    for(int i = 0; i < points.size(); i++) {
        if(points[i].value > minimum) {
            points[i].value = std::max(minimum, points[i].value - decay);
            thunder_decision ++;
        }
    }

    fireworks->set_active(thunder_decision > 10);
    thunder->set_active(thunder_decision > 8);
    rain->set_intensity(thunder_decision / 2);

    for(int v = 0; v < seq->CHANNELS; v++) {
        ToneGenerator * voice = seq->voices[v];
        if(voice->get_parameter(ToneGenerator::PARAMETER_ACTIVE)) {
            int f = voice->get_parameter(ToneGenerator::PARAMETER_FREQUENCY);
            int d = std::max(2, voice->get_parameter(ToneGenerator::PARAMETER_DUTY));
            d /= 2;
            f *= d;

            int bin = 0;
            if(f >= max_freq) bin = points.size() - 1;
            else if(f > min_freq) {
                int hz_per_div = (max_freq - min_freq) / points.size();
                bin = (f / hz_per_div) % points.size();
                bin = std::min(std::max(0, bin), (int) points.size() - 1);
            }
            points[bin].value = std::min(maximum, points[bin].value + rise);
            if(bin > 0 && points[bin - 1].value < (maximum / 4)) points[bin - 1].value = std::min(maximum, points[bin].value + rise/3);
            if(bin < (points.size() - 1) && points[bin + 1].value < (maximum / 4)) points[bin + 1].value = std::min(maximum, points[bin].value + rise/3);
        }
    }

    if(hid_test_key_state(KEY_UP) == KEYSTATE_HIT) {
        // Prev track
        if(trackNo == minTrackNo) trackNo = maxTrackNo;
        else trackNo--;
        load_and_play();
    }
    else if(hid_test_key_state(KEY_DOWN) == KEYSTATE_HIT) {
        // Next track
        if(trackNo == maxTrackNo) trackNo = minTrackNo;
        else trackNo++;
        load_and_play();
    }
    else if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
        // Exit
        pop_state(STATE_MUSICBOX, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
    }

    WeatherChartCommon::step();
    if(trackNo != all_chime_count + 1) // Allow hiding only if MIDI IN mode
        hint_framecounter = 255;
}

void AppShimMusicbox::cleanup() {
    seq->stop_sequence();
    WeatherChartCommon::cleanup();
}

void AppShimMusicbox::load_and_play() {
    if(seq->is_sequencing()) seq->stop_sequence();
    if(trackNo == all_chime_count + 1) {
        // Randomize is MIDI visualizer in this context
        hint_lbl->set_string("MIDI In");
        hint_lbl->hidden = false;
        hint_lbl->stopped = true;
        hint_lbl->x_offset = DisplayFramebuffer::width / 2 - hint_lbl->string_width / 2;
        hint_framecounter = 0;
    }
    else {
        auto melody = melody_from_no(trackNo);
        hint_lbl->set_string(melody->get_title());
        hint_lbl->x_offset = 0;
        seq->play_sequence(melody, SEQUENCER_REPEAT_INDEFINITELY);
        hint_lbl->stopped = false;
        hint_lbl->hidden = false;
    }
}
