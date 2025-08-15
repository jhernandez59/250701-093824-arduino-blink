// ------------------------------
// Archivo: eeprom.h
// ------------------------------
#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>

#define SIZE_NOMBRE 50
#define SIZE_LAT_LON 20
#define SIZE_ALTURA 20

struct ConfigWM {
  char nombre[SIZE_NOMBRE];
  char lat[SIZE_LAT_LON];
  char lon[SIZE_LAT_LON];
  char alt[SIZE_ALTURA];
};

// --- Tamaño total de la EEPROM
// Usamos sizeof() para calcularlo automáticamente
#define EEPROM_SIZE sizeof(ConfigWM)

bool saveConfigEEPROM(const ConfigWM& config);
bool loadConfigEEPROM(ConfigWM& config);
void clearConfigEEPROM();

#endif