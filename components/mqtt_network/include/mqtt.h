/*------------------------------------------------------------*-
  MQTT (over TCP) - header file
  (c) Minh-An Dao 2020
  version 1.00 - 09/10/2020
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
#define TOPIC_DIR     "trai" CONFIG_FARM_NAME "/chuong" CONFIG_BARN_NAME "/thietbi" CONFIG_DEVICE_NAME
#define CMD_TOPIC     TOPIC_DIR "/cmd"
#define STATUS_TOPIC  TOPIC_DIR "/status"
#define DATA_TOPIC    TOPIC_DIR "/data"
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