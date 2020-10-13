/*------------------------------------------------------------*-
  Indicating LED - header file
  (c) Minh-An Dao 2020
  version 1.00 - 13/10/2020
---------------------------------------------------------------
 * Setup timer for LED.
 * 
 --------------------------------------------------------------*/
#ifndef __LED_H
#define __LED_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ------ Public constants ------------------------------------
// ------ Public function prototypes --------------------------
/**
 * @brief Connect to MQTT broker
 * @return ESP_OK on successful connection
 */
void led_init(void);
void led_lit(void);
void led_off(void);
void led_blink(void);

// ------ Public variable -------------------------------------

#ifdef __cplusplus
}
#endif

#endif