/*------------------------------------------------------------*-
  NETWORK - header file
  (c) Minh-An Dao 2020
  version 1.00 - 09/10/2020
---------------------------------------------------------------
 * Configuration to connect to Wifi and Ethernet.
 * 
 * ref: esp-idf\examples\common_components\protocol_examples_common
 *      https://github.com/espressif/esp-idf/issues/894 
 --------------------------------------------------------------*/
#ifndef __NETWORK_H
#define __NETWORK_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

// ------ Public constants ------------------------------------
// #ifdef CONFIG_CONNECT_ETHERNET
// #define NETWORK_INTERFACE get_netif()
// #endif

// #ifdef CONFIG_CONNECT_WIFI
// #define NETWORK_INTERFACE get_netif()
// #endif
// ------ Public function prototypes --------------------------
/**
 * @brief network initialize function - needed for any net interface
 */
esp_err_t network_init(void);
/**
 * @brief Connect to Wi-Fi and/or Ethernet, wait for IP
 * @return ESP_OK on successful connection
 */
esp_err_t network_start(void);
/**
 * Counterpart to connect, de-initializes Wi-Fi or Ethernet
 */
esp_err_t network_stop(void);
// /**
//  * @brief Returns esp-netif pointer created by connect()
//  *
//  * @note If multiple interfaces active at once, this API return NULL
//  * In that case the get_netif_from_desc() should be used
//  * to get esp-netif pointer based on interface description
//  */
// esp_netif_t *get_netif(void);
// /**
//  * @brief Returns esp-netif pointer created by connect() described by
//  * the supplied desc field
//  *
//  * @param desc Textual interface of created network interface, for example "sta"
//  * indicate default WiFi station, "eth" default Ethernet interface.
//  *
//  */
// esp_netif_t *get_netif_from_desc(const char *desc);

// ------ Public variable -------------------------------------
// extern SemaphoreHandle_t baton;

#ifdef __cplusplus
}
#endif

#endif