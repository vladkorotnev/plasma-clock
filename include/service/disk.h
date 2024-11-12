#pragma once
#include <os_config.h>
#include <FS.h>

#ifndef FS_MOUNTPOINT
#define FS_MOUNTPOINT "/disk"
#endif

bool mount_disk(bool format_if_failed = true);
void unmount_disk();
/// @brief For Arduino library use only. Otherwise try using normal C file APIs.
File open_file(const char * name, const char * mode = "r");
