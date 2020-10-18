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
bool preference_begin(const char * name, bool readOnly=false);
void preference_end();

bool preference_clear();
bool preference_remove(const char * key);

size_t preference_putChar(const char* key, int8_t value);
size_t preference_putUChar(const char* key, uint8_t value);
size_t preference_putShort(const char* key, int16_t value);
size_t preference_putUShort(const char* key, uint16_t value);
size_t preference_putInt(const char* key, int32_t value);
size_t preference_putUInt(const char* key, uint32_t value);
size_t preference_putLong(const char* key, int32_t value);
size_t preference_putULong(const char* key, uint32_t value);
size_t preference_putLong64(const char* key, int64_t value);
size_t preference_putULong64(const char* key, uint64_t value);
size_t preference_putFloat(const char* key, float_t value);
size_t preference_putDouble(const char* key, double_t value);
size_t preference_putBool(const char* key, bool value);
size_t preference_putString(const char* key, const char* value);
size_t preference_putString(const char* key, String value);
size_t preference_putBytes(const char* key, const void* value, size_t len);

int8_t  preference_getChar(const char* key, int8_t defaultValue = 0);
uint8_t preference_getUChar(const char* key, uint8_t defaultValue = 0);
int16_t preference_getShort(const char* key, int16_t defaultValue = 0);
uint16_t preference_getUShort(const char* key, uint16_t defaultValue = 0);
int32_t preference_getInt(const char* key, int32_t defaultValue = 0);
uint32_t preference_getUInt(const char* key, uint32_t defaultValue = 0);
int32_t preference_getLong(const char* key, int32_t defaultValue = 0);
uint32_t preference_getULong(const char* key, uint32_t defaultValue = 0);
int64_t preference_getLong64(const char* key, int64_t defaultValue = 0);
uint64_t preference_getULong64(const char* key, uint64_t defaultValue = 0);
float_t preference_getFloat(const char* key, float_t defaultValue = NAN);
double_t preference_getDouble(const char* key, double_t defaultValue = NAN);
bool preference_getBool(const char* key, bool defaultValue = false);
size_t preference_getString(const char* key, char* value, size_t maxLen);
String preference_getString(const char* key, String defaultValue = String());
size_t preference_getBytesLength(const char* key);
size_t preference_getBytes(const char* key, void * buf, size_t maxLen);
size_t preference_freeEntries();

// ------ Public variable -------------------------------------

#ifdef __cplusplus
}
#endif

#endif