# POMF format music folder

All of the cpp files in here must adhere to the following rules:

* Have `extern "C" const POMFHeader POMF_HEAD` with the song metadata
* Have `extern "C" const melody_item_t POMF_TUNE[]` with the song events

They will then be converted into the [POmf](../../include/sound/pomf.h) format by [ELF2POmf](../../helper/elf2pomf.py) and placed into the `data` folder to be inserted into SPIFFS.

## Template

```c
#include <sound/pomf.h>

extern "C" const POMFHeader POMF_HEAD = {
    POMF_MAGIC_FILE,
    POMF_CURVER,
    "ShortTitle",
    "Long Title"
};

extern "C" const melody_item_t POMF_TUNE[] = {
};
```