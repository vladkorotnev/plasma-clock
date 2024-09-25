#pragma once
#include <stdint.h>
#include <sound/generators.h>
#include <sound/sequencer.h>

/*
.                           ____
.    ____  ____  ____ ___  / __/
.   / __ \/ __ \/ __ `__ \/ /_  
.  / /_/ / /_/ / / / / / / __/       :3c
. / .___/\____/_/ /_/ /_/_/     
./_/                            

    POrtable Music Format

This is a janky attempt to make a portable module format so that:
    1. The songs compile from within the CPP files of this project
        (so as to make sure there is never a divergence between the data expected and the data in the files)
    2. It's entirely compatible with the existing sequencer format

Format in a nutshell:
    POMFHeader(magic = ASCII 'POmf')
    [
        1 or more of POMFChunk:
            - magic (4 bytes)
            - size (4 bytes LE, excluding magic and size itself)
            - if magic is 'saMP':
                - rle_sample_t followed by .length bytes of sample data
            - if magic is 'tuNE':
                - .size bytes of melody_item_t data
    ]
 */

#define POMF_MAGIC_FILE 0x666D4F50 // 'POmf' little-endian
#define POMF_MAGIC_SAMPLE 0x504D6173 // 'saMP'
#define POMF_MAGIC_TRACK 0x454E7574 // 'tuNE'

#define POMF_CURVER_MAJ (1)
#define POMF_CURVER_MIN (0)
#define POMF_CURVER ((POMF_CURVER_MAJ << 8) | (POMF_CURVER_MIN))

struct POMFChunk {
    const uint32_t magic;
    const uint32_t size;
    union {
        struct {
            const rle_sample_t sample_header;
            const uint8_t sample_data[];
        } sample_chunk;
        struct {
            const melody_item_t event_data[];
        } tune_chunk;
    } data;
};

struct __attribute__((packed)) POMFHeader {
    const uint32_t magic;
    const uint16_t version;
    const char short_title[32];
    const char long_title[64];
};
