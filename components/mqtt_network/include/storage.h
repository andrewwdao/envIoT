/*------------------------------------------------------------*-
  Indicating PREFERENCE - header file
  (c) Minh-An Dao 2020
  version 1.00 - 13/10/2020
---------------------------------------------------------------
 * Save important data in NVS - non volatile storage memory (in flash! not EEPROM, perfect!)
 * Ported from Cpp Preference library of arduino-esp32.
 * 
 * ref: https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences
 * 
 --------------------------------------------------------------*/
#ifndef __PREFERENCE_H
#define __PREFERENCE_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

// ------ Public constants ------------------------------------
// ------ Public function prototypes --------------------------
/**
 * @brief Init indicating PREFERENCE
 */
void store_WifiCredentials(char* WSSID, char* WPASS);
void del_WifiCredentials(void);
bool read_WifiSSID(char* ssid);
bool read_WifiPass(char* pass);

// ------ Public variable -------------------------------------

#ifdef __cplusplus
}
#endif

#endif