#include <Arduino.h>

String getChipId() {
  uint64_t macAddress = ESP.getEfuseMac();
  uint32_t id         = 0;

  for (int i = 0; i < 17; i = i + 8) {
    id |= ((macAddress >> (40 - i)) & 0xff) << i;
  }
  return String(id, HEX);
}


void hexDump(const char* tag, const uint8_t* pData, uint32_t length) {
    // credit: https://github.com/nkolban/esp32-snippets/blob/fe3d318acddf87c6918944f24e8b899d63c816dd/cpp_utils/GeneralUtils.cpp#L299
    char ascii[80];
    char hex[80];
    char tempBuf[80];
    uint32_t lineNumber = 0;

    ESP_LOGV(tag, "     00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    ESP_LOGV(tag, "     -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
    strcpy(ascii, "");
    strcpy(hex, "");
    uint32_t index = 0;
    while (index < length) {
        sprintf(tempBuf, "%.2x ", pData[index]);
        strcat(hex, tempBuf);
        if (isprint(pData[index])) {
            sprintf(tempBuf, "%c", pData[index]);
        } else {
            sprintf(tempBuf, ".");
        }
        strcat(ascii, tempBuf);
        index++;
        if (index % 16 == 0) {
            ESP_LOGV(tag, "%.4x %s %s", lineNumber * 16, hex, ascii);
            strcpy(ascii, "");
            strcpy(hex, "");
            lineNumber++;
        }
    }
    if (index %16 != 0) {
        while (index % 16 != 0) {
            strcat(hex, "   ");
            index++;
        }
        ESP_LOGV(tag, "%.4x %s %s", lineNumber * 16, hex, ascii);
    }
} // hexDump
