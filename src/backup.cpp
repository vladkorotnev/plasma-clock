#include <backup.h>
#include <esp_partition.h>
#include <esp32-hal-log.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static char LOG_TAG[] = "BAK";

static portMUX_TYPE prefs_write_spinlock = portMUX_INITIALIZER_UNLOCKED;
static size_t prefs_write_pointer = 0;
static const esp_partition_t *prefs_write_partition = nullptr;

static const char* get_type_str(esp_partition_type_t type)
{
    switch(type) {
        case ESP_PARTITION_TYPE_APP:
            return "ESP_PARTITION_TYPE_APP";
        case ESP_PARTITION_TYPE_DATA:
            return "ESP_PARTITION_TYPE_DATA";
        default:
            return "UNKNOWN_PARTITION_TYPE"; 
    }
}

static const char* get_subtype_str(esp_partition_subtype_t subtype)
{
    switch(subtype) {
        case ESP_PARTITION_SUBTYPE_DATA_NVS:
            return "ESP_PARTITION_SUBTYPE_DATA_NVS";
        case ESP_PARTITION_SUBTYPE_DATA_PHY:
            return "ESP_PARTITION_SUBTYPE_DATA_PHY";
        case ESP_PARTITION_SUBTYPE_APP_FACTORY:
            return "ESP_PARTITION_SUBTYPE_APP_FACTORY";
        case ESP_PARTITION_SUBTYPE_DATA_FAT:
            return "ESP_PARTITION_SUBTYPE_DATA_FAT";
        default:
            return "UNKNOWN_PARTITION_SUBTYPE";
    }
}

static bool mount_partition(esp_partition_type_t type, esp_partition_subtype_t subtype, const char* name, spi_flash_mmap_handle_t * out_handle, const void ** out_ptr, size_t* out_size)
{
    ESP_LOGI(LOG_TAG, "Find partition with type %s, subtype %s, label %s...", get_type_str(type), get_subtype_str(subtype),
                    name == NULL ? "NULL (unspecified)" : name);

    const esp_partition_t * part  = esp_partition_find_first(type, subtype, name);

    if (part != NULL) {
        ESP_LOGI(LOG_TAG, "\tfound partition '%s' at offset 0x%" PRIx32 " with size 0x%" PRIx32, part->label, part->address, part->size);
        *out_size = part->size;
        esp_err_t rslt = esp_partition_mmap(part, 0, part->size, SPI_FLASH_MMAP_DATA, out_ptr, out_handle);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG,"\tmount failed: %i (%s)", rslt, esp_err_to_name(rslt));
            return false;
        } else {
            return true;
        }
    } else {
        ESP_LOGE(LOG_TAG, "\tpartition not found!");
        return false;
    }
}

void unmount_partition(spi_flash_mmap_handle_t handle) {
    spi_flash_munmap(handle);
}

bool mount_settings(const void** out_ptr, partition_handle_t* out_handle, size_t* out_size) {
    return mount_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, nullptr, out_handle, out_ptr, out_size);
}

bool mount_crash(const void** out_ptr, partition_handle_t* out_handle, size_t* out_size) {
    bool rslt = mount_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, nullptr, out_handle, out_ptr, out_size);
    if(rslt) {
        // Offset to produce a valid ELF
        *out_ptr += 0x14;
        *out_size -= 0x14;
    }
    return rslt;
}

bool begin_settings_write() {
    prefs_write_pointer = 0;
    prefs_write_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, nullptr);
    if(prefs_write_partition != nullptr) {
        esp_err_t rslt = esp_partition_erase_range(prefs_write_partition, 0, prefs_write_partition->size);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "Erase failed: %i (%s)", rslt, esp_err_to_name(rslt));
        }
        return true;
    } 

    return false;
}

void write_settings_chunk(const char * chunk, size_t size) {
    if(prefs_write_partition != nullptr) {
        esp_err_t rslt = esp_partition_write(prefs_write_partition, prefs_write_pointer, (const void*)chunk, size);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "Write failed: %i (%s)", rslt, esp_err_to_name(rslt));
        }
        prefs_write_pointer += size;
    }
}

void end_settings_write() {
    if(prefs_write_partition != nullptr) {
        prefs_write_partition = nullptr;
    }
}