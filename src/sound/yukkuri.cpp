#include <sound/yukkuri.h>
#include <service/prefs.h>

static char LOG_TAG[] = "AQTK";

Yukkuri::Yukkuri(const char * license, uint16_t frame_length) {
    #if HAS(AQUESTALK)
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);
        ESP_LOGI(LOG_TAG, "MAC = %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        uint8_t rslt = CAqTkPicoF_Init(workbuf, frame_length, license);
        if(rslt) {
            ESP_LOGE(LOG_TAG, "Init error (%i)", rslt);
        } else {
            pcm_buf_size = frame_length;
            pcm_buf = (int16_t*) calloc(sizeof(int16_t), pcm_buf_size);
            ready = true;
            ESP_LOGI(LOG_TAG, "Init OK");
        }
    #endif
}

size_t Yukkuri::fill_buffer(void* buffer, size_t length) {
#if HAS(AQUESTALK)
    if(!speaking || !ready) {
        return 0;
    }

    uint8_t* buff = (uint8_t*) buffer;
    uint32_t want_samples = length * 8;
    size_t idx = 0;
    int bit = 7;

    for(int s = 0; s < want_samples; s++) {
        if(out_phase == STRETCH_FACTOR) {
            // We finished the current PCM sample, move to the next one
            pcm_playhead++;
            if(pcm_playhead >= pcm_buf_length) {
                // We finished the whole PCM buffer, generate a new one
                int rslt = CAqTkPicoF_SyntheFrame((int16_t*)pcm_buf, &pcm_buf_length);
                if(rslt || pcm_buf_length == 0) {
                    // No samples were generated, end of phrase
                    _finish_current();
                    break; // <- if there is another phrase it will start on the next buffer fill
                } else {
                    pcm_playhead = 0;
                    out_phase = 0;
                    if(out_state && pcm_buf[pcm_playhead] <= HYST_ZERO_MARGIN) {
                        out_state = false;
                    }
                    else if(!out_state && pcm_buf[pcm_playhead] >= HYST_ONE_MARGIN) {
                        out_state = true;
                    }
                }
            } else {
                out_phase = 0;
                if(out_state && pcm_buf[pcm_playhead] <= HYST_ZERO_MARGIN) {
                    out_state = false;
                }
                else if(!out_state && pcm_buf[pcm_playhead] >= HYST_ONE_MARGIN) {
                    out_state = true;
                }
            }
        }

        idx = s / 8;
        bit = 8 - (s % 8);
        if(out_state) {
            buff[idx] |= (1 << bit);
        }
        out_phase++;
    }
    
    return idx + 1;
#else
    return 0;
#endif
}

bool Yukkuri::speak(YukkuriUtterance& utterance) {
#if HAS(AQUESTALK)
    if(!ready) return false;

    if(queue.size() < QUEUE_MAX_LENGTH) {
        queue.push(utterance);
        if(!speaking) {
            _start_next_utterance_if_needed();
        }
        return true;
    } else {
        ESP_LOGE(LOG_TAG, "Cannot speak [%s]: no space in queue", utterance.text);
        return false;
    }
#else
    return false;
#endif
}

bool Yukkuri::speak(const char * text, YukkuriUtteranceCallback callback) {
    YukkuriUtterance u = {
        text, callback, 0xFF, 0xFFFFu
    };
    return speak(u);
}

void Yukkuri::cancel_current() {
    if(queue.size() > 0) {
        ESP_LOGI(LOG_TAG, "Cancel: [%s]", queue.front().text);
        queue.front().callback(false);
        queue.pop();
    }
    _start_next_utterance_if_needed();
}

void Yukkuri::cancel_all() {
    while(queue.size() > 0) {
        queue.front().callback(false);
        queue.pop();
    }
    ESP_LOGI(LOG_TAG, "Cancelled all");
    _stop_speech();
}

void Yukkuri::_finish_current() {
    if(queue.size() > 0) {
        ESP_LOGV(LOG_TAG, "Finish: [%s]", queue.front().text);
        queue.front().callback(true);
        queue.pop();
    }
    _start_next_utterance_if_needed();
}

void Yukkuri::_stop_speech() {
    speaking = false;
    out_state = false;
}

void Yukkuri::_start_next_utterance_if_needed() {
    if(queue.size() > 0) {
        YukkuriUtterance next = queue.front();
#if HAS(AQUESTALK)
        if(next.speed == 0xFF) {
            next.speed = prefs_get_int(PREFS_KEY_VOICE_SPEED);
            if(next.speed == 0) next.speed = 100;
        }
        int rslt = CAqTkPicoF_SetKoe((const uint8_t*)next.text, next.speed, next.pause);
        if(rslt) {
            ESP_LOGE(LOG_TAG, "ERROR(%i): [%s]", rslt, next.text);
        } else {
            ESP_LOGI(LOG_TAG, "Start: [%s]", next.text);
            speaking = true;
        }
#endif
    } else {
        ESP_LOGV(LOG_TAG, "Utterance queue is now empty");
        _stop_speech();
    }
}