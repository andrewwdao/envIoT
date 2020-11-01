/*------------------------------------------------------------*-
  MQTT (SSL) - header file
  (c) Minh-An Dao 2020
  version 1.10 - 15/10/2020
---------------------------------------------------------------
 * Configuration to connect to MQTT Broker.
 * 
 --------------------------------------------------------------*/
#ifndef __MQTT_H
#define __MQTT_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

// ------ Public constants ------------------------------------
#define MQTT_HOST     "mqtts://" CONFIG_IOTHUBNAME ".azure-devices.net"
#define MQTT_USERNAME CONFIG_IOTHUBNAME ".azure-devices.net/" CONFIG_DEVICEID "/?api-version=2018-06-30"
#define MQTT_PASSWORD CONFIG_MQTT_PASSWORD
#define TOPIC_DIR     "devices/" CONFIG_DEVICEID "/messages/events/"//"trai" CONFIG_FARM_NAME "/chuong" CONFIG_BARN_NAME "/thietbi" CONFIG_DEVICE_NAME
#define CMD_TOPIC     "devices/" CONFIG_DEVICEID "/messages/devicebound/#" //TOPIC_DIR "/cmd"
#define LWT_TOPIC     TOPIC_DIR  //TOPIC_DIR "/status"
#define DATA_TOPIC    TOPIC_DIR  //TOPIC_DIR "/data"

// ------ Public function prototypes --------------------------
/**
 * @brief Connect to MQTT broker
 * @return ESP_OK on successful connection
 */
esp_err_t mqtt_start(void);
/**
 * @brief Subscribe to a topic in MQTT broker
 * @return msg_id on successful connection
 */
int mqtt_sub(const char *topic, int qos);
/**
 * @brief Unsubscribe to a topic in MQTT broker
 * @return msg_id on successful connection
 */
int mqtt_unsub(const char *topic);
/**
 * @brief Publish a message to a topic in MQTT broker
 * @return msg_id on successful publishment
 */
int mqtt_pub(const char *topic, const char *data, int qos, int retain);
// ------ Public variable -------------------------------------

#ifdef __cplusplus
}
#endif

#endif