#include <graphics/screenshooter.h>
#include <esp32-hal-log.h>
#include <PNGenc.h>

static char LOG_TAG[] = "SCAP";

Screenshooter::Screenshooter() {
}

bool Screenshooter::capture(FantaManipulator * fb, const uint8_t **outBufAddr, size_t* outBufLen) {
    const size_t scratch_size = 4096;
    uint8_t * scratch = nullptr;
    uint8_t * line_scratch = nullptr;
    uint16_t * src_buf;
    static uint8_t ucPal[768] = {0,0,0,255,255,255};
    static PNG encoder;
    int rslt, y, x, w, h;
    size_t dataLen;

    scratch = (uint8_t*) malloc(scratch_size); // not gralloc here as this won't be rendered!
    if(scratch == nullptr) {
        ESP_LOGE(LOG_TAG, "Failure when allocating PNG scratch buffer");    
        goto free_shit_and_bail;
    }

    line_scratch = (uint8_t*) malloc(fb->get_width());
    if(line_scratch == nullptr) {
        ESP_LOGE(LOG_TAG, "Failure when allocating line scratch buffer");    
        goto free_shit_and_bail;
    }

    rslt = encoder.open(scratch, scratch_size);
    if(rslt != PNG_SUCCESS) {
        ESP_LOGE(LOG_TAG, "Error %i initializing PNG encoder", rslt);
        goto free_shit_and_bail;
    }

    w = fb->get_width();
    h = fb->get_height();

    rslt = encoder.encodeBegin(w, h, PNG_PIXEL_INDEXED, 8, ucPal, 3);
    if(rslt != PNG_SUCCESS) {
        ESP_LOGE(LOG_TAG, "Error %i initializing PNG encoder", rslt);
        goto free_shit_and_bail;
    }

    if(!fb->lock()) {
        ESP_LOGE(LOG_TAG, "Could not lock source buffer");
        goto free_shit_and_bail;
    }

    src_buf = (uint16_t*) fb->get_fanta();

    for(y = 0; y < h && rslt == PNG_SUCCESS; y++) {
        for(x = 0; x < w; x++) {
            line_scratch[x] = ((src_buf[x] & (1 << y)) != 0) ? 1 : 0;
        }
        rslt = encoder.addLine(line_scratch);
    }

    fb->unlock();

    if(rslt != PNG_SUCCESS) {
        ESP_LOGE(LOG_TAG, "Encoder failed");
        goto free_shit_and_bail;
    }

    dataLen = encoder.close();
    ESP_LOGI(LOG_TAG, "Encoded %i bytes", dataLen);

    if(dataLen < scratch_size) {
        scratch = (uint8_t*) realloc(scratch, dataLen);
    }

    *outBufAddr = scratch;
    *outBufLen = dataLen;
    free(line_scratch);

    return true;

free_shit_and_bail:
    if(scratch != nullptr) free(scratch);
    if(line_scratch != nullptr) free(line_scratch);
    *outBufLen = 0;
    return false;
}