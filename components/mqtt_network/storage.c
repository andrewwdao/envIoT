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
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "storage.h"


// ------ Private constants -----------------------------------
const char * nvs_errors[] = { "OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENGHT"};
#define nvs_error(e) (((e)>ESP_ERR_NVS_BASE)?nvs_errors[(e)&~(ESP_ERR_NVS_BASE)]:nvs_errors[0])
// ------ Private function prototypes -------------------------
// ------ Private variables -----------------------------------
/** @brief tag used for ESP serial console messages */
static const char *TAG = "STORAGE";
// ------ PUBLIC variable definitions -------------------------
//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
static esp_err_t __nvs_begin(nvs_handle_t *nvs_handle, const char *name, bool read_only)
{
    esp_err_t err = nvs_open(name, read_only?NVS_READONLY:NVS_READWRITE, nvs_handle);
    if(err)
    {
        ESP_LOGE(TAG, "nvs_open failed: %s", nvs_error(err));
        //return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t __nvs_end(nvs_handle_t nvs_handle)
{
    nvs_close(nvs_handle);
    return ESP_OK;
}

static esp_err_t __nvs_clear(nvs_handle_t nvs_handle)
{
    esp_err_t err = nvs_erase_all(nvs_handle);
    if(err)
    {
        ESP_LOGE(TAG, "nvs_erase_all failed: %s", nvs_error(err));
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t __nvs_putString(nvs_handle_t nvs_handle, const char* key, char* value)
{
    if(!key || !value)  
    {
        ESP_LOGE(TAG, "invalid argument");
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t err = nvs_set_str(nvs_handle, key, value);
    if(err){
        ESP_LOGE(TAG, "nvs_set_str fail: %s %s", key, nvs_error(err));
        return ESP_FAIL;
    }
    err = nvs_commit(nvs_handle);
    if(err){
        ESP_LOGE(TAG, "nvs_commit fail: %s %s", key, nvs_error(err));
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t __nvs_putBool(nvs_handle_t nvs_handle, const char* key, const uint8_t value)
{
    if(!key)
    {
        ESP_LOGE(TAG, "invalid argument");
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t err = nvs_set_u8(nvs_handle, key, (value?1:0));
    if(err){
        ESP_LOGE(TAG, "nvs_set_u8 fail: %s %s", key, nvs_error(err));
        return ESP_FAIL;
    }
    err = nvs_commit(nvs_handle);
    if(err){
        ESP_LOGE(TAG, "nvs_commit fail: %s %s", key, nvs_error(err));
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t __nvs_getString(nvs_handle_t nvs_handle, const char* key, char* value, const size_t maxLen)
{
    if(!key || !value)  
    {
        ESP_LOGE(TAG, "invalid argument");
        return ESP_ERR_INVALID_ARG;
    }
    size_t len = 0;
    esp_err_t err = nvs_get_str(nvs_handle, key, NULL, &len);
    if(err){
        ESP_LOGE(TAG, "nvs_get_str len fail: %s %s", key, nvs_error(err));
        //return ESP_FAIL;
    }
    if(len > maxLen){
        ESP_LOGE(TAG, "not enough space in value: %u < %u", maxLen, len);
        return ESP_ERR_INVALID_SIZE;
    }
    err = nvs_get_str(nvs_handle, key, value, &len);
    if(err){
        ESP_LOGE(TAG, "nvs_get_str fail: %s %s", key, nvs_error(err));
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t __nvs_getBool(nvs_handle_t nvs_handle, const char* key, uint8_t* value, const uint8_t defaultValue)
{
    if(!key)
    {
        ESP_LOGE(TAG, "invalid argument");
        return ESP_ERR_INVALID_ARG;
    }
    *value = defaultValue;
    esp_err_t err = nvs_get_u8(nvs_handle, key, value);
    if(err){
        ESP_LOGE(TAG, "nvs_get_u8 fail: %s %s", key, nvs_error(err));
        //return ESP_FAIL;
    }
    return ESP_OK;
}

void store_WifiCredentials(char* WSSID, char* WPASS) {
    /**
     * @param handle: distinguisher
     * @param namespace: Namespace name is limited to 15 chars
     * @param readonly: bool.
     */
    nvs_handle_t my_handle=0;
    ESP_ERROR_CHECK(__nvs_begin(&my_handle, "WiFiInfo", false)); //handle, namespace, readonly
    ESP_ERROR_CHECK(__nvs_putString(my_handle,"WSSID", WSSID));
    ESP_ERROR_CHECK(__nvs_putString(my_handle,"WPASS", WPASS));
    ESP_ERROR_CHECK(__nvs_putBool(my_handle,"valid", true));
    ESP_ERROR_CHECK(__nvs_end(my_handle));
}
void del_WifiCredentials() {
    nvs_handle_t my_handle=0;
    ESP_ERROR_CHECK(__nvs_begin(&my_handle, "WiFiInfo", false)); //handle, namespace, readonly
    ESP_ERROR_CHECK(__nvs_clear(my_handle));
	ESP_ERROR_CHECK(__nvs_end(my_handle));
}
//maxlength of ssid is 32 bytes
bool read_WifiSSID(char* ssid) {
    nvs_handle_t my_handle=0;
    ESP_ERROR_CHECK(__nvs_begin(&my_handle, "WiFiInfo", true)); //handle, namespace, readonly
    uint8_t valid_data = false;
    ESP_ERROR_CHECK(__nvs_getBool(my_handle, "valid", &valid_data, false)); //handle, key, buffer, default value
    if (valid_data) {
        ESP_ERROR_CHECK(__nvs_getString(my_handle, "WSSID", ssid, 33)); //handle, key, pointer, maxlength 
        ESP_ERROR_CHECK(__nvs_end(my_handle));
        return true;
    } else {
		ESP_ERROR_CHECK(__nvs_end(my_handle));
        snprintf(ssid, 4, "N/A");
        return false;
    }
}
//maxlength of password is 64 bytes
bool read_WifiPass(char* pass) {
    nvs_handle_t my_handle=0;
    ESP_ERROR_CHECK(__nvs_begin(&my_handle, "WiFiInfo", true)); //handle, namespace, readonly
    uint8_t valid_data = false;
    ESP_ERROR_CHECK(__nvs_getBool(my_handle, "valid", &valid_data, false)); //handle, key, buffer, default value
    if (valid_data) {
        ESP_ERROR_CHECK(__nvs_getString(my_handle, "WPASS", pass, 65)); //handle, key, pointer, maxlength 
        ESP_ERROR_CHECK(__nvs_end(my_handle));
        return true;
    } else {
		ESP_ERROR_CHECK(__nvs_end(my_handle));
        snprintf(pass, 4, "N/A");
        return false;
    }
}
