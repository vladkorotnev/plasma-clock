
#include <rsrc/common_icons.h>

/*
Based upon the Mega 1-bit Icon Bundle by PiiXL:
https://piiixl.itch.io/mega-1-bit-icons-bundle

If you're in this folder to rip out the assets - please go to the link above and purchase a copy of the pack.

*/

const uint8_t alarm_icns_data[] = {
0x39, 0xce, 0x70, 0x87, 0x67, 0xf3, 0x48, 0x09, 0x13, 0x64, 0x27, 0xf2, 0x2f, 0xda, 0x2f, 0xba, 
0x24, 0x72, 0x2f, 0xfa, 0x2f, 0xfa, 0x27, 0xf2, 0x13, 0x64, 0x08, 0x08, 0x17, 0xf4, 0x20, 0x02
};

const sprite_t alarm_icns = {
    .width = 16,
    .height = 16,
    .data = alarm_icns_data,
    .mask = nullptr,
    .format = SPRFMT_HORIZONTAL
};

const uint8_t good_icns_data[] = {
    0x00, 0x00, 0x00, 0x80, 0x00, 0xc0, 0x01, 0xc0, 0x03, 0xc0, 0x07, 0x80, 0x0f, 0xfc, 0x7f, 0xfe, 
    0x7f, 0xfe, 0x7f, 0xfa, 0x7f, 0xfc, 0x7f, 0xfc, 0x7f, 0xf4, 0x7f, 0xf8, 0x7f, 0xe8, 0x0f, 0xf0
};

const sprite_t good_icns = { .width = 16, .height = 16, .data = good_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

const uint8_t hourglass_icns_data[] = {
    0x3f, 0xfe, 0x00, 0x00, 0x10, 0x04, 0x10, 0x04, 0x10, 0x04, 0x0b, 0xe8, 0x05, 0xd0, 0x02, 0xa0, 
    0x02, 0x20, 0x04, 0x10, 0x08, 0x88, 0x11, 0xc4, 0x12, 0xe4, 0x15, 0x74, 0x00, 0x00, 0x3f, 0xfe
};

const sprite_t hourglass_icns = { .width = 16, .height = 16, .data = hourglass_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

const uint8_t icon_thermo_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x10, 0x00, 0x38, 0x00, 0x74, 0x00, 0xfe, 0x00, 0x7d, 0x00, 0x38, 0x80, 0x11, 0x40, 0x08, 0x20, 
    0x04, 0x5c, 0x02, 0x02, 0x01, 0x0d, 0x00, 0x8d, 0x00, 0x81, 0x00, 0x81, 0x00, 0x42, 0x00, 0x3c
};

const sprite_t icon_thermo_1616 = {
    .width = 16,
    .height = 16,
    .data = icon_thermo_data,
    .mask = nullptr,
    .format = SPRFMT_HORIZONTAL
};

const uint8_t icn_music_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x07, 0xfc, 0x07, 0xfc, 0x07, 0xc4, 0x04, 0x04, 0x04, 0x04, 
    0x04, 0x04, 0x04, 0x3c, 0x3c, 0x74, 0x74, 0x64, 0x64, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00
};

const sprite_t music_icns = {
    .width = 16, .height = 16, .data = icn_music_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL
};
