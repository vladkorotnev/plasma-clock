#include <service/disk.h>
#include <os_config.h>
#include <LittleFS.h>

bool mount_disk(bool format_if_needed) {
    return LittleFS.begin(format_if_needed, FS_MOUNTPOINT);
}

void unmount_disk() {
    LittleFS.end();
}

File open_file(const char * name, const char * mode) {
    return LittleFS.open(name, mode);
}
