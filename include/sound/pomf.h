#pragma once
#include <stdint.h>
#include <sound/types.h>

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
            - if magic is 'saZZ' or 'tuZZ':
                - contents of 'saMP' or 'tuNE' respectively, compressed with DEFLATE with a -15 bit window.
                  `size` shows the length of the compressed data, `realsize` the length of the target buffer for uncompression.
    ]
    POMFChunk(size = 0, magic = ASCII 'eof ')

Version history:
    1.0: Initial version
    1.1: Add primitive compression support (saZZ, tuZZ chunks)
 */

#define POMF_MAGIC_FILE 0x666D4F50 // 'POmf' little-endian
#define POMF_MAGIC_SAMPLE 0x504D6173 // 'saMP'
#define POMF_MAGIC_SAMPLE_COMPRESSED 0x5A5A6173 // 'saZZ'
#define POMF_MAGIC_TRACK 0x454E7574 // 'tuNE'
#define POMF_MAGIC_TRACK_COMPRESSED 0x5A5A7574 // 'tuZZ'
#define POMF_MAGIC_END 0x20666F65 // 'eof '

#define POMF_CURVER_MAJ (1)
#define POMF_CURVER_MIN (1)
#define POMF_CURVER ((POMF_CURVER_MAJ << 8) | (POMF_CURVER_MIN))

struct POMFChunkHeader {
    const uint32_t magic;
    const uint32_t size;
    const uint32_t realsize;
};

struct __attribute__((packed)) POMFHeader {
    const uint32_t magic;
    const uint16_t version;
    const char short_title[32];
    const char long_title[64];
};
