/*------------------------------------------------------------*-
  Indicating SENSOR - header file
  (c) Minh-An Dao 2020
  version 1.00 - 13/10/2020
---------------------------------------------------------------
 * Setup timer for SENSOR.
 * 
 --------------------------------------------------------------*/
#ifndef __SENSOR_H
#define __SENSOR_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

// ------ Public constants ------------------------------------
// ------ Public function prototypes --------------------------
/**
 * @brief sensor init function (public)
 * will automatically send data through mqtt protocol
 */
void sensor_init(void);
/**
 * @brief sensor stop function (public)
 */
void sensor_stop(void);
// ------ Public variable -------------------------------------

#ifdef __cplusplus
}
#endif

#endif