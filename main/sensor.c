/*------------------------------------------------------------*-
  NETWORK - header file
  (c) 2017 David Antliff. Licensed under the MIT license.
  (c) Minh-An Dao 2020
  version 1.00 - 09/10/2020
---------------------------------------------------------------
 * Configure and read from the DS18B20 temperature sensor.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "sensor.h"
#include "temp_sensor.h"

// ------ Private constants -----------------------------------
#define SECRET_STOPKEY       (74)
#define ONE_WIRE_GPIO        (CONFIG_ONE_WIRE_GPIO)
#define MAX_TEMP_SENSORS     (CONFIG_MAX_TEMP_SENSORS)
#define TEMP_RESOLUTION      (DS18B20_RESOLUTION_12_BIT)
#define SAMPLE_PERIOD        (CONFIG_SAMPLE_PERIOD)   // ms
// ------ Private function prototypes -------------------------
// ------ Private variables -----------------------------------
/** @brief tag used for ESP serial console messages */
static const char *TAG = "SENSOR";
xQueueHandle _sensor_stop_queue;
// ------ PUBLIC variable definitions -------------------------
//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
/**
 * @brief sensor stop function
 */
static void __sensor_task(void* arg)
{
    /** @warning Stable readings require a brief period before communication */
    // vTaskDelay(2000.0 / portTICK_PERIOD_MS);
    // To debug, use 'make menuconfig' to set default Log level to DEBUG, then uncomment:
    //esp_log_level_set("owb", ESP_LOG_DEBUG);
    //esp_log_level_set("ds18b20", ESP_LOG_DEBUG);

    
    // Create a 1-Wire bus, using the RMT timeslot driver
    OneWireBus * _owb;
    owb_rmt_driver_info rmt_driver_info;
    _owb = owb_rmt_initialize(&rmt_driver_info, ONE_WIRE_GPIO, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(_owb, true);  // enable CRC check for ROM code

    // Find all connected devices
    ESP_LOGI(TAG, "Find devices:");
    OneWireBus_ROMCode device_rom_codes[MAX_TEMP_SENSORS] = {0};
    uint8_t num_devices = 0;
    OneWireBus_SearchState search_state = {0};
    bool found = false;
    owb_search_first(_owb, &search_state, &found);
    while (found)
    {
        char rom_code_s[17];
        owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
        ESP_LOGI(TAG, "  %d : %s", num_devices, rom_code_s);
        device_rom_codes[num_devices] = search_state.rom_code;
        ++num_devices;
        owb_search_next(_owb, &search_state, &found);
    }
    ESP_LOGI(TAG, "Found %d device%s", num_devices, num_devices == 1 ? "" : "s");

    // If a single device is present, then the ROM code is probably
    // not very interesting, so just print it out. If there are multiple devices,
    // then it may be useful to check that a specific device is present.

    if (num_devices == 1)
    {
        // For a single device only:
        OneWireBus_ROMCode rom_code;
        owb_status status = owb_read_rom(_owb, &rom_code);
        if (status == OWB_STATUS_OK)
        {
            char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
            owb_string_from_rom_code(rom_code, rom_code_s, sizeof(rom_code_s));
            ESP_LOGI(TAG, "Single device %s present", rom_code_s);
        }
        else
        {
            ESP_LOGE(TAG, "An error occurred reading ROM code: %d", status);
        }
    }
    else
    {
        // Search for a known ROM code (LSB first):
        // For example: 0x1502162ca5b2ee28
        OneWireBus_ROMCode known_device = {
            .fields.family = { 0x28 },
            .fields.serial_number = { 0xee, 0xb2, 0xa5, 0x2c, 0x16, 0x02 },
            .fields.crc = { 0x15 },
        };
        char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
        owb_string_from_rom_code(known_device, rom_code_s, sizeof(rom_code_s));
        bool is_present = false;

        owb_status search_status = owb_verify_rom(_owb, known_device, &is_present);
        if (search_status == OWB_STATUS_OK)
        {
            ESP_LOGI(TAG, "Device %s is %s", rom_code_s, is_present ? "present" : "not present");
        }
        else
        {
            ESP_LOGE(TAG, "An error occurred searching for known device: %d", search_status);
        }
    }

    // Create DS18B20 devices on the 1-Wire bus
    DS18B20_Info * _sensors[MAX_TEMP_SENSORS] = {0};
    for (int i = 0; i < num_devices; ++i)
    {
        DS18B20_Info * buf_device = ds18b20_malloc();  // heap allocation
        _sensors[i] = buf_device;

        if (num_devices == 1)
        {
            ESP_LOGI(TAG, "Single device optimisations enabled");
            ds18b20_init_solo(buf_device, _owb);          // only one device on bus
        }
        else
        {
            ds18b20_init(buf_device, _owb, device_rom_codes[i]); // associate with bus and device
        }
        ds18b20_use_crc(buf_device, true);           // enable CRC check on all reads
        ds18b20_set_resolution(buf_device, TEMP_RESOLUTION);
    }

    // Check for parasitic-powered devices
    bool parasitic_power = false;
    ds18b20_check_for_parasite_power(_owb, &parasitic_power);
    if (parasitic_power) {
        ESP_LOGI(TAG, "Parasitic-powered devices detected");
    }
    // In parasitic-power mode, devices cannot indicate when conversions are complete,
    // so waiting for a temperature conversion must be done by waiting a prescribed duration
    owb_use_parasitic_power(_owb, parasitic_power);

#ifdef CONFIG_ENABLE_STRONG_PULLUP_GPIO
    // An external pull-up circuit is used to supply extra current to OneWireBus devices
    // during temperature conversions.
    owb_use_strong_pullup_gpio(_owb, CONFIG_STRONG_PULLUP_GPIO);
#endif

    // Read temperatures more efficiently by starting conversions on all devices at the same time
    // int errors_count[MAX_TEMP_SENSORS] = {0};
    if (num_devices > 0)
    {
        TickType_t last_wake_time = xTaskGetTickCount();
        uint8_t stop_signal;
        while (1)
        {
            /** @note signal to delete the task */
            if(xQueueReceive(_sensor_stop_queue, &stop_signal, 0)) //no waiting, just checking for any news
            {
                if (stop_signal == SECRET_STOPKEY) //if stop signal is the secret code
                {
                    // clean up dynamically allocated data
                    for (int i = 0; i < num_devices; ++i) ds18b20_free(&_sensors[i]);
                    owb_uninitialize(_owb);
                    ESP_LOGI(TAG, "Sensor stopped.");
                    vTaskDelete(NULL); //delete itself
                }
            }
            last_wake_time = xTaskGetTickCount();

            ds18b20_convert_all(_owb);

            // In this application all devices use the same resolution,
            // so use the first device to determine the delay
            ds18b20_wait_for_conversion(_sensors[0]);

            // Read the results immediately after conversion otherwise it may fail
            // (using printf before reading may take too long)
            float readings[MAX_TEMP_SENSORS] = { 0 };
            DS18B20_ERROR sensor_event[MAX_TEMP_SENSORS] = { 0 };

            for (int i = 0; i < num_devices; ++i) sensor_event[i] = ds18b20_read_temp(_sensors[i], &readings[i]);

            // Print results in a separate loop, after all have been read
            for (int i = 0; i < num_devices; ++i)
            {
                if (sensor_event[i] == DS18B20_OK)
                {
                    ESP_LOGW(TAG, " - Temperature %d: %.1f (oC)", i+1, readings[i]);
                }
            }

            vTaskDelayUntil(&last_wake_time, SAMPLE_PERIOD / portTICK_PERIOD_MS);
        }
    }
    else
    {
        ESP_LOGE(TAG, "\nNo DS18B20 devices detected!\n");
    }
}
/**
 * @brief sensor init function (public)
 * will automatically send data through mqtt protocol
 */
void sensor_init(void)
{
    _sensor_stop_queue = xQueueCreate(1, sizeof(uint8_t));
    //------------ sensor task -----------------
    xTaskCreate(
        &__sensor_task,    /* Task Function */
        "sensor task",   /* Name of Task */
        2046,           /* Stack size of Task */
        NULL,           /* Parameter of the task */
        1,              /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
        NULL);          /* Task handle to keep track of created task */
}
/**
 * @brief sensor stop function (public)
 */
void sensor_stop(void)
{
    // send signal for the task to delete itself
    uint8_t stop_signal = SECRET_STOPKEY;
    xQueueSend(_sensor_stop_queue, &stop_signal,  portMAX_DELAY);
}
