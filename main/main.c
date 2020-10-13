/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "network.h"
#include "mqtt.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "esp_netif.h"
// ------ Private constants -----------------------------------
/**
 * @note config parameters via "idf.py menuconfig
 */
#define DELAY_MS(...)     {vTaskDelay(__VA_ARGS__/portTICK_RATE_MS);}

// ------ Private function prototypes -------------------------

// ------ Private variables -----------------------------------
/** @brief tag used for ESP serial console messages */
static const char *TAG = "main";

// ------ PUBLIC variable definitions -------------------------

//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
/**
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information, should not be ever started on production code! This is an example on how you can integrate your code with wifi-manager
 */
void monitoring_task(void *arg)
{
	for(;;){
		ESP_LOGI(TAG, "free heap: %d\n",esp_get_free_heap_size());
		DELAY_MS(1000);
	}
}

void app_main(void)
{
    // --------Initialize NVS - must have on main task -------
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    //-------------- Network task ------------------
    // xTaskCreate(
    //     &network_task,    /* Task Function */
    //     "network_task",   /* Name of Task */
    //     4096,          /* Stack size of Task */
    //     NULL,           /* Parameter of the task */
    //     3,              /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
    //     NULL);          /* Task handle to keep track of created task */

    //------------ monitoring task -----------------
    //A task on core 2 that monitors free heap memory - should be removed on production
    xTaskCreatePinnedToCore(
        &monitoring_task,    /* Task Function */
        "monitoring_task",   /* Name of Task */
        2048,                /* Stack size of Task */
        NULL,                /* Parameter of the task */
        1,                   /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
        NULL,                /* Task handle to keep track of created task */
        1);                  /* CoreID */
    
    network_init();
    //for (;;){
    //    network_start();
    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

        ESP_LOGW(TAG, "Hellooooooooooooo wifi");
        mqtt_start();
        mqtt_sub(STATUS_TOPIC, 1);
        //for (;;) {
            //DELAY_MS(3000);
            //mqtt_pub(CMD_TOPIC, "HELLOOOOOO",1,0);
            //DELAY_MS(3000);
            //mqtt_pub(STATUS_TOPIC, "who are you???",1,0);
        //}
        // DELAY_MS(5000);
        // network_stop();
        // ESP_LOGW(TAG, "Byebyeeeeeeeeeeee wifi");
        // DELAY_MS(5000);
    //}
    
}
