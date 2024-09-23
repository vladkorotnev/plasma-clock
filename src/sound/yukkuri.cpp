#include <sound/yukkuri.h>
#include <service/prefs.h>

static char LOG_TAG[] = "AQTK";

static SemaphoreHandle_t aqtkSemaphore = NULL;

Yukkuri::Yukkuri(const char * license, uint16_t frame_length) {
    #if HAS(AQUESTALK)
        if(aqtkSemaphore == NULL) {
            aqtkSemaphore = xSemaphoreCreateBinary();
            xSemaphoreGive(aqtkSemaphore);
        }
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);
        ESP_LOGI(LOG_TAG, "MAC = %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI(LOG_TAG, "License: %s", license == nullptr ? "TRIAL" : license);
        memset(workbuf, 0, AQ_SIZE_WORKBUF);
        xSemaphoreTake(aqtkSemaphore, portMAX_DELAY);
        uint8_t rslt = CAqTkPicoF_Init(workbuf, frame_length, license);
        xSemaphoreGive(aqtkSemaphore);
        if(rslt) {
            ESP_LOGE(LOG_TAG, "Init error (%i)", rslt);
        } else {
            pcm_buf = (int16_t*) calloc(sizeof(int16_t), frame_length);
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
                pcm_buf_length = 0;
                xSemaphoreTake(aqtkSemaphore, portMAX_DELAY);
                int rslt = CAqTkPicoF_SyntheFrame((int16_t*)pcm_buf, &pcm_buf_length);
                xSemaphoreGive(aqtkSemaphore);
                if(rslt) {
                    // End of phrase
                    ESP_LOGE(LOG_TAG, "CAqTkPicoF_SyntheFrame rslt=%i, buf_len = %i", rslt, pcm_buf_length);
                    _finish_current();
                    if(pcm_buf_length == 0) break;
                }
                pcm_playhead = 0;
                out_phase = 0;
            } else {
                out_phase = 0;
            }
        }

        if(out_phase == 0) {
            if(old_style_resampling) {
                if(out_state && pcm_buf[pcm_playhead] <= HYST_ZERO_MARGIN) {
                    out_state = false;
                }
                else if(!out_state && pcm_buf[pcm_playhead] >= HYST_ONE_MARGIN) {
                    out_state = true;
                }
            } else {
                if(pcm_buf[pcm_playhead] != 0) {
                    // I tried a LUT here but it's too slow, AQTK starts dropping words and outright segfaulting
                    // This seems to be sufficiently fast, sufficiently precise and overall good enough
                    int32_t sample = (std::max((int32_t)-8192, std::min((int32_t)8192, (int32_t) pcm_buf[pcm_playhead] / 3)) + 8192) * 12 / 16384;
                    out_ones = (sample % 12);
                    out_zeros = 11 - out_ones;
                } else {
                    out_zeros = 11;
                    out_ones = 0;
                }
            }
        }

        if(!old_style_resampling) {
            if(out_ones == 0) {
                out_state = false;
            }
            else if(out_zeros == 0) {
                out_state = true;
            }
            else if(out_phase % ((out_state ? out_zeros : out_ones)) == 0) {
                out_state ^= 1;
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
        utterance.callback(false);
        return false;
    }
#else
    utterance.callback(false);
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
        old_style_resampling = (prefs_get_int(PREFS_KEY_VOICE_MODE_RESAMPLING) == 1);
        xSemaphoreTake(aqtkSemaphore, portMAX_DELAY);
        int rslt = CAqTkPicoF_SetKoe((const uint8_t*)next.text, next.speed, next.pause);
        xSemaphoreGive(aqtkSemaphore);
        if(rslt) {
            ESP_LOGE(LOG_TAG, "ERROR(%i): [%s]", rslt, next.text);
            cancel_current();
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

bool Yukkuri::is_speaking() {
    return speaking;
}
