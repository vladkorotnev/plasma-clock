#pragma once
#include <stdint.h>
#include <strings.h>

typedef uint32_t partition_handle_t;

void unmount_partition(partition_handle_t);
bool mount_settings(const void** out_ptr, partition_handle_t* out_handle, size_t* out_size);
bool mount_crash(const void** out_ptr, partition_handle_t* out_handle, size_t* out_size);

bool begin_settings_write();
void write_settings_chunk(const char *, size_t);
void end_settings_write();