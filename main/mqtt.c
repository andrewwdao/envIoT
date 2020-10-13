/*------------------------------------------------------------*-
  MQTT (over TCP) - header file
  (c) Minh-An Dao 2020
  version 1.00 - 09/10/2020
---------------------------------------------------------------
 * Configuration to connect to MQTT Broker.
 * 
 --------------------------------------------------------------*/
#ifndef __MQTT_C
#define __MQTT_C
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"

// ------ Private constants -----------------------------------
// ------ Private function prototypes -------------------------
// ------ Private variables -----------------------------------
/** @brief tag used for ESP serial console messages */
static const char *TAG = "MQTT";
esp_mqtt_client_handle_t _client;
// ------ PUBLIC variable definitions -------------------------
//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
/**
 * @brief MQTT event handler
 * @note mostly used for subscribe topic handling
 */

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    esp_mqtt_client_handle_t client = event->client;
    // int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGW(TAG, "MQTT Connected!");
            esp_mqtt_client_subscribe(client,CMD_TOPIC,1); //client, topic, qos
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGW(TAG, "Subscribed, msg_id=%d", event->msg_id);
            ESP_LOGW(TAG, " - Topic: %.*s", event->topic_len, event->topic);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGW(TAG, "Unsubscribed, msg_id=%d", event->msg_id);
            ESP_LOGW(TAG, " - Topic: %.*s", event->topic_len, event->topic);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Published, msg_id=%d", event->msg_id);
            ESP_LOGI(TAG, " - Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, " - Data: %.*s", event->data_len, event->data);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGW(TAG, "Cmd Received from: %.*s", event->topic_len, event->topic);
            ESP_LOGW(TAG, " - Cmd: %.*s", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}
int mqtt_sub(const char *topic, int qos)
{
    return esp_mqtt_client_subscribe(_client, topic, qos); //client, topic, qos
}
int mqtt_pub(const char *topic, const char *data, int qos, int retain)
{
    return esp_mqtt_client_publish(_client, topic, data, 0, qos, retain); //client, topic, data, len, qos, retain  
}
esp_err_t mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    _client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(_client, ESP_EVENT_ANY_ID, mqtt_event_handler, _client);
    esp_mqtt_client_start(_client);
    return ESP_OK;
}

#endif