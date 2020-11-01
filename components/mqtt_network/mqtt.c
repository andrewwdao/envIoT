/*------------------------------------------------------------*-
  MQTT (SSL) - header file
  (c) Minh-An Dao 2020
  version 1.10 - 15/10/2020
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
#include "esp_event.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"
#include "led.h"

// ------ Private constants -----------------------------------
// ------ Private function prototypes -------------------------
// ------ Private variables -----------------------------------
/** @brief tag used for ESP serial console messages */
static const char *TAG = "MQTT";
esp_mqtt_client_handle_t _client;
uint8_t MQTT_CONNECTED_FLAG = 0;
// ------ PUBLIC variable definitions -------------------------
extern const uint8_t cert_pem_start[]   asm("_binary_cert_pem_start");
extern const uint8_t cert_pem_end[]     asm("_binary_cert_pem_end");
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
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGW(TAG, "MQTT Connected!");
            led_lit();
            MQTT_CONNECTED_FLAG = 1;
            esp_mqtt_client_publish(client, LWT_TOPIC, "1", 0, 1, 0); //client, topic, data, len, qos, retain  
            esp_mqtt_client_subscribe(client, CMD_TOPIC, 1); //client, topic, qos
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected!");
            led_blink();
            MQTT_CONNECTED_FLAG = 0;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGW(TAG, " - Subscribed, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGW(TAG, " - Unsubscribed, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGW(TAG, " - Published, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGW(TAG, "Cmd Received from: %.*s", event->topic_len, event->topic);
            ESP_LOGW(TAG, " - Cmd: %.*s", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT event Error!");
            led_off();
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT event before connect");
            break;
        default:
            break;
    }
}
int mqtt_sub(const char *topic, int qos)
{
    if (MQTT_CONNECTED_FLAG){
        int msg_id = esp_mqtt_client_subscribe(_client, topic, qos); //client, topic, qos
        ESP_LOGW(TAG, "Subscribing to: %.*s", strlen(topic), topic);
        return msg_id;
    }
    return -1;
}
int mqtt_unsub(const char *topic)
{
    if (MQTT_CONNECTED_FLAG){
        int msg_id = esp_mqtt_client_unsubscribe(_client, topic); //client, topic, qos
        ESP_LOGW(TAG, "Unsubscribing to: %.*s", strlen(topic), topic);
        return msg_id;
    }
    return -1;
}
int mqtt_pub(const char *topic, const char *data, int qos, int retain)
{
    if (MQTT_CONNECTED_FLAG){
        int msg_id = esp_mqtt_client_publish(_client, topic, data, 0, qos, retain); //client, topic, data, len, qos, retain  
        ESP_LOGW(TAG, "Publishing to: %.*s", strlen(topic), topic);
        ESP_LOGW(TAG, " - Data: %.*s", strlen(data), data);
        return msg_id;
    }
    return -1;
}
esp_err_t mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        // .host = CONFIG_BROKER_HOST,
        // .port = CONFIG_BROKER_PORT,
        // .username = CONFIG_MQTT_USERNAME,
        // .password = CONFIG_MQTT_PASSWORD,
        .uri = MQTT_HOST,
        .port = 8883,
        .username = MQTT_USERNAME,
        .password = MQTT_PASSWORD,
        .lwt_topic = LWT_TOPIC,
        .lwt_msg = "0",
        .lwt_msg_len = 1,
        .lwt_qos = 1,
        .lwt_retain = 0,
        .keepalive = 120,
        .client_id = CONFIG_DEVICEID,
        .cert_pem = (const char *)cert_pem_start,

    };
    _client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(_client, ESP_EVENT_ANY_ID, mqtt_event_handler, _client);
    esp_mqtt_client_start(_client);
    return ESP_OK;
}

#endif