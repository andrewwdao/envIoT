/*------------------------------------------------------------*-
  Indicating LED - header file
  (c) Minh-An Dao 2020
  version 1.00 - 13/10/2020
---------------------------------------------------------------
 * Setup LED for indicating network status.
 * Based on ledc lib.
 * 
 --------------------------------------------------------------*/
#include <stdlib.h>
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "preferences.h"


// ------ Private constants -----------------------------------
const char * nvs_errors[] = { "OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENGHT"};
#define nvs_error(e) (((e)>ESP_ERR_NVS_BASE)?nvs_errors[(e)&~(ESP_ERR_NVS_BASE)]:nvs_errors[0])
// ------ Private function prototypes -------------------------
// ------ Private variables -----------------------------------
static uint32_t _handle=0;
static bool _started=false;
static bool _readOnly=false;
// ------ PUBLIC variable definitions -------------------------
//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
bool preference_begin(const char * name, bool readOnly){
    if(_started)    return false;
    _readOnly = readOnly;
    esp_err_t err = nvs_open(name, readOnly?NVS_READONLY:NVS_READWRITE, &_handle);
    if(err){
        log_e("nvs_open failed: %s", nvs_error(err));
        return false;
    }
    _started = true;
    return true;
}

void preference_end(){
    if(!_started)   return;
    nvs_close(_handle);
    _handle = 0;
    _started = false;
}
/**
 * @brief Clear all keys in opened preferences
 */
bool preference_clear(){
    if(!_started || _readOnly)  return false;
    esp_err_t err = nvs_erase_all(_handle);
    if(err){
        log_e("nvs_erase_all fail: %s", nvs_error(err));
        return false;
    }
    return    true;

}
/**
 * @brief Remove a key
 */
bool preference_remove(const char * key){
    if(!_started || !key || _readOnly)  return false;
    esp_err_t err = nvs_erase_key(_handle, key);
    if(err){
        log_e("nvs_erase_key fail: %s %s", key, nvs_error(err));
        return false;
    }
    return true;
}
/**
 * @brief Put a key value
 */
size_t preference_putChar(const char* key, int8_t value){
    if(!_started || !key || _readOnly)  return 0;
    esp_err_t err = nvs_set_i8(_handle, key, value);
    if(err){
        log_e("nvs_set_i8 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 1;
}
/**
 * @brief Put an unsigned key value
 */
size_t preference_putUChar(const char* key, uint8_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u8(_handle, key, value);
    if(err){
        log_e("nvs_set_u8 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 1;
}
size_t preference_putShort(const char* key, int16_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_i16(_handle, key, value);
    if(err){
        log_e("nvs_set_i16 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 2;
}

size_t preference_putUShort(const char* key, uint16_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u16(_handle, key, value);
    if(err){
        log_e("nvs_set_u16 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 2;
}

size_t preference_putInt(const char* key, int32_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_i32(_handle, key, value);
    if(err){
        log_e("nvs_set_i32 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 4;
}

size_t preference_putUInt(const char* key, uint32_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u32(_handle, key, value);
    if(err){
        log_e("nvs_set_u32 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 4;
}

size_t preference_putLong(const char* key, int32_t value){
    return putInt(key, value);
}

size_t preference_putULong(const char* key, uint32_t value){
    return putUInt(key, value);
}

size_t preference_putLong64(const char* key, int64_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_i64(_handle, key, value);
    if(err){
        log_e("nvs_set_i64 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 8;
}

size_t preference_putULong64(const char* key, uint64_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u64(_handle, key, value);
    if(err){
        log_e("nvs_set_u64 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 8;
}
static size_t __putString(const char* key, const char* value){
    if(!_started || !key || !value || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_str(_handle, key, value);
    if(err){
        log_e("nvs_set_str fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return strlen(value);
}

size_t preference_putString(const char* key, const String value){
    return __putString(key, value.c_str());
}

size_t preference_putBytes(const char* key, const void* value, size_t len){
    if(!_started || !key || !value || !len || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_blob(_handle, key, value, len);
    if(err){
        log_e("nvs_set_blob fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

size_t preference_putFloat(const char* key, const float_t value){
    return putBytes(key, (void*)&value, sizeof(float_t));
}

size_t preference_putDouble(const char* key, const double_t value){
    return putBytes(key, (void*)&value, sizeof(double_t));
}

size_t preference_putBool(const char* key, const bool value){
    return putUChar(key, (uint8_t) (value ? 1 : 0));
}


/*
 * Get a key value
 * */

int8_t preference_getChar(const char* key, const int8_t defaultValue){
    int8_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i8(_handle, key, &value);
    if(err){
        log_v("nvs_get_i8 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint8_t preference_getUChar(const char* key, const uint8_t defaultValue){
    uint8_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u8(_handle, key, &value);
    if(err){
        log_v("nvs_get_u8 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

int16_t preference_getShort(const char* key, const int16_t defaultValue){
    int16_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i16(_handle, key, &value);
    if(err){
        log_v("nvs_get_i16 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint16_t preference_getUShort(const char* key, const uint16_t defaultValue){
    uint16_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u16(_handle, key, &value);
    if(err){
        log_v("nvs_get_u16 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

int32_t preference_getInt(const char* key, const int32_t defaultValue){
    int32_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i32(_handle, key, &value);
    if(err){
        log_v("nvs_get_i32 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint32_t preference_getUInt(const char* key, const uint32_t defaultValue){
    uint32_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u32(_handle, key, &value);
    if(err){
        log_v("nvs_get_u32 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

int32_t preference_getLong(const char* key, const int32_t defaultValue){
    return getInt(key, defaultValue);
}

uint32_t preference_getULong(const char* key, const uint32_t defaultValue){
    return getUInt(key, defaultValue);
}

int64_t preference_getLong64(const char* key, const int64_t defaultValue){
    int64_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i64(_handle, key, &value);
    if(err){
        log_v("nvs_get_i64 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint64_t preference_getULong64(const char* key, const uint64_t defaultValue){
    uint64_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u64(_handle, key, &value);
    if(err){
        log_v("nvs_get_u64 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

float_t preference_getFloat(const char* key, const float_t defaultValue) {
    float_t value = defaultValue;
    getBytes(key, (void*) &value, sizeof(float_t));
    return value;
}

double_t preference_getDouble(const char* key, const double_t defaultValue) {
    double_t value = defaultValue;
    getBytes(key, (void*) &value, sizeof(double_t));
    return value;
}

bool preference_getBool(const char* key, const bool defaultValue) {
    return getUChar(key, defaultValue ? 1 : 0) == 1;
}

size_t preference_getString(const char* key, char* value, const size_t maxLen){
    size_t len = 0;
    if(!_started || !key || !value || !maxLen){
        return 0;
    }
    esp_err_t err = nvs_get_str(_handle, key, NULL, &len);
    if(err){
        log_e("nvs_get_str len fail: %s %s", key, nvs_error(err));
        return 0;
    }
    if(len > maxLen){
        log_e("not enough space in value: %u < %u", maxLen, len);
        return 0;
    }
    err = nvs_get_str(_handle, key, value, &len);
    if(err){
        log_e("nvs_get_str fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

String preference_getString(const char* key, const String defaultValue){
    char * value = NULL;
    size_t len = 0;
    if(!_started || !key){
        return String(defaultValue);
    }
    esp_err_t err = nvs_get_str(_handle, key, value, &len);
    if(err){
        log_e("nvs_get_str len fail: %s %s", key, nvs_error(err));
        return String(defaultValue);
    }
    char buf[len];
    value = buf;
    err = nvs_get_str(_handle, key, value, &len);
    if(err){
        log_e("nvs_get_str fail: %s %s", key, nvs_error(err));
        return String(defaultValue);
    }
    return String(buf);
}

size_t preference_getBytesLength(const char* key){
    size_t len = 0;
    if(!_started || !key){
        return 0;
    }
    esp_err_t err = nvs_get_blob(_handle, key, NULL, &len);
    if(err){
        log_e("nvs_get_blob len fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

size_t preference_getBytes(const char* key, void * buf, size_t maxLen){
    size_t len = getBytesLength(key);
    if(!len || !buf || !maxLen){
        return len;
    }
    if(len > maxLen){
        log_e("not enough space in buffer: %u < %u", maxLen, len);
        return 0;
    }
    esp_err_t err = nvs_get_blob(_handle, key, buf, &len);
    if(err){
        log_e("nvs_get_blob fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

size_t preference_freeEntries() {
    nvs_stats_t nvs_stats;
    esp_err_t err = nvs_get_stats(NULL, &nvs_stats);
    if(err){
        log_e("Failed to get nvs statistics");
        return 0;
    }
    return nvs_stats.free_entries;
}


#include "nvs.h"

const char * nvs_errors[] = { "OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENGHT"};
#define nvs_error(e) (((e)>ESP_ERR_NVS_BASE)?nvs_errors[(e)&~(ESP_ERR_NVS_BASE)]:nvs_errors[0])

Preferences::Preferences()
    :_handle(0)
    ,_started(false)
    ,_readOnly(false)
{}

Preferences::~Preferences(){
    end();
}

bool Preferences::begin(const char * name, bool readOnly){
    if(_started){
        return false;
    }
    _readOnly = readOnly;
    esp_err_t err = nvs_open(name, readOnly?NVS_READONLY:NVS_READWRITE, &_handle);
    if(err){
        log_e("nvs_open failed: %s", nvs_error(err));
        return false;
    }
    _started = true;
    return true;
}

void Preferences::end(){
    if(!_started){
        return;
    }
    nvs_close(_handle);
    _started = false;
}

/*
 * Clear all keys in opened preferences
 * */

bool Preferences::clear(){
    if(!_started || _readOnly){
        return false;
    }
    esp_err_t err = nvs_erase_all(_handle);
    if(err){
        log_e("nvs_erase_all fail: %s", nvs_error(err));
        return false;
    }
    return true;
}

/*
 * Remove a key
 * */

bool Preferences::remove(const char * key){
    if(!_started || !key || _readOnly){
        return false;
    }
    esp_err_t err = nvs_erase_key(_handle, key);
    if(err){
        log_e("nvs_erase_key fail: %s %s", key, nvs_error(err));
        return false;
    }
    return true;
}

/*
 * Put a key value
 * */

size_t Preferences::putChar(const char* key, int8_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_i8(_handle, key, value);
    if(err){
        log_e("nvs_set_i8 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 1;
}

size_t Preferences::putUChar(const char* key, uint8_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u8(_handle, key, value);
    if(err){
        log_e("nvs_set_u8 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 1;
}

size_t Preferences::putShort(const char* key, int16_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_i16(_handle, key, value);
    if(err){
        log_e("nvs_set_i16 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 2;
}

size_t Preferences::putUShort(const char* key, uint16_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u16(_handle, key, value);
    if(err){
        log_e("nvs_set_u16 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 2;
}

size_t Preferences::putInt(const char* key, int32_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_i32(_handle, key, value);
    if(err){
        log_e("nvs_set_i32 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 4;
}

size_t Preferences::putUInt(const char* key, uint32_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u32(_handle, key, value);
    if(err){
        log_e("nvs_set_u32 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 4;
}

size_t Preferences::putLong(const char* key, int32_t value){
    return putInt(key, value);
}

size_t Preferences::putULong(const char* key, uint32_t value){
    return putUInt(key, value);
}

size_t Preferences::putLong64(const char* key, int64_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_i64(_handle, key, value);
    if(err){
        log_e("nvs_set_i64 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 8;
}

size_t Preferences::putULong64(const char* key, uint64_t value){
    if(!_started || !key || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_u64(_handle, key, value);
    if(err){
        log_e("nvs_set_u64 fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return 8;
}

size_t Preferences::putFloat(const char* key, const float_t value){
    return putBytes(key, (void*)&value, sizeof(float_t));
}

size_t Preferences::putDouble(const char* key, const double_t value){
    return putBytes(key, (void*)&value, sizeof(double_t));
}

size_t Preferences::putBool(const char* key, const bool value){
    return putUChar(key, (uint8_t) (value ? 1 : 0));
}

size_t Preferences::putString(const char* key, const char* value){
    if(!_started || !key || !value || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_str(_handle, key, value);
    if(err){
        log_e("nvs_set_str fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return strlen(value);
}

size_t Preferences::putString(const char* key, const String value){
    return putString(key, value.c_str());
}

size_t Preferences::putBytes(const char* key, const void* value, size_t len){
    if(!_started || !key || !value || !len || _readOnly){
        return 0;
    }
    esp_err_t err = nvs_set_blob(_handle, key, value, len);
    if(err){
        log_e("nvs_set_blob fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

/*
 * Get a key value
 * */

int8_t Preferences::getChar(const char* key, const int8_t defaultValue){
    int8_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i8(_handle, key, &value);
    if(err){
        log_v("nvs_get_i8 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint8_t Preferences::getUChar(const char* key, const uint8_t defaultValue){
    uint8_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u8(_handle, key, &value);
    if(err){
        log_v("nvs_get_u8 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

int16_t Preferences::getShort(const char* key, const int16_t defaultValue){
    int16_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i16(_handle, key, &value);
    if(err){
        log_v("nvs_get_i16 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint16_t Preferences::getUShort(const char* key, const uint16_t defaultValue){
    uint16_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u16(_handle, key, &value);
    if(err){
        log_v("nvs_get_u16 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

int32_t Preferences::getInt(const char* key, const int32_t defaultValue){
    int32_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i32(_handle, key, &value);
    if(err){
        log_v("nvs_get_i32 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint32_t Preferences::getUInt(const char* key, const uint32_t defaultValue){
    uint32_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u32(_handle, key, &value);
    if(err){
        log_v("nvs_get_u32 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

int32_t Preferences::getLong(const char* key, const int32_t defaultValue){
    return getInt(key, defaultValue);
}

uint32_t Preferences::getULong(const char* key, const uint32_t defaultValue){
    return getUInt(key, defaultValue);
}

int64_t Preferences::getLong64(const char* key, const int64_t defaultValue){
    int64_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_i64(_handle, key, &value);
    if(err){
        log_v("nvs_get_i64 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

uint64_t Preferences::getULong64(const char* key, const uint64_t defaultValue){
    uint64_t value = defaultValue;
    if(!_started || !key){
        return value;
    }
    esp_err_t err = nvs_get_u64(_handle, key, &value);
    if(err){
        log_v("nvs_get_u64 fail: %s %s", key, nvs_error(err));
    }
    return value;
}

float_t Preferences::getFloat(const char* key, const float_t defaultValue) {
    float_t value = defaultValue;
    getBytes(key, (void*) &value, sizeof(float_t));
    return value;
}

double_t Preferences::getDouble(const char* key, const double_t defaultValue) {
    double_t value = defaultValue;
    getBytes(key, (void*) &value, sizeof(double_t));
    return value;
}

bool Preferences::getBool(const char* key, const bool defaultValue) {
    return getUChar(key, defaultValue ? 1 : 0) == 1;
}

size_t Preferences::getString(const char* key, char* value, const size_t maxLen){
    size_t len = 0;
    if(!_started || !key || !value || !maxLen){
        return 0;
    }
    esp_err_t err = nvs_get_str(_handle, key, NULL, &len);
    if(err){
        log_e("nvs_get_str len fail: %s %s", key, nvs_error(err));
        return 0;
    }
    if(len > maxLen){
        log_e("not enough space in value: %u < %u", maxLen, len);
        return 0;
    }
    err = nvs_get_str(_handle, key, value, &len);
    if(err){
        log_e("nvs_get_str fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

String Preferences::getString(const char* key, const String defaultValue){
    char * value = NULL;
    size_t len = 0;
    if(!_started || !key){
        return String(defaultValue);
    }
    esp_err_t err = nvs_get_str(_handle, key, value, &len);
    if(err){
        log_e("nvs_get_str len fail: %s %s", key, nvs_error(err));
        return String(defaultValue);
    }
    char buf[len];
    value = buf;
    err = nvs_get_str(_handle, key, value, &len);
    if(err){
        log_e("nvs_get_str fail: %s %s", key, nvs_error(err));
        return String(defaultValue);
    }
    return String(buf);
}

size_t Preferences::getBytesLength(const char* key){
    size_t len = 0;
    if(!_started || !key){
        return 0;
    }
    esp_err_t err = nvs_get_blob(_handle, key, NULL, &len);
    if(err){
        log_e("nvs_get_blob len fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

size_t Preferences::getBytes(const char* key, void * buf, size_t maxLen){
    size_t len = getBytesLength(key);
    if(!len || !buf || !maxLen){
        return len;
    }
    if(len > maxLen){
        log_e("not enough space in buffer: %u < %u", maxLen, len);
        return 0;
    }
    esp_err_t err = nvs_get_blob(_handle, key, buf, &len);
    if(err){
        log_e("nvs_get_blob fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

size_t Preferences::freeEntries() {
    nvs_stats_t nvs_stats;
    esp_err_t err = nvs_get_stats(NULL, &nvs_stats);
    if(err){
        log_e("Failed to get nvs statistics");
        return 0;
    }
    return nvs_stats.free_entries;
}