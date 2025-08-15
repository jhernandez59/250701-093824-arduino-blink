// eeprom.cpp
#include "eeprom.h"
#include <EEPROM.h>

bool saveConfigEEPROM(const ConfigWM& config) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(0, config);
  bool exito = EEPROM.commit();
  EEPROM.end();
  return exito;
}

bool loadConfigEEPROM(ConfigWM& config) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, config);
  EEPROM.end();
  if (strlen(config.nombre) == 0 || strlen(config.lat) == 0 ||
      strlen(config.lon) == 0 || strlen(config.alt) == 0) {
    Serial.println("⚠️ EEPROM vacía o inválida, usando valores por defecto");
    return false;
  }
  return true;
}

// Función para borrar la configuración
void clearConfigEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  for (size_t i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);  // Llenar con ceros
  }
  bool success = EEPROM.commit();
  EEPROM.end();

  if (success) {
    Serial.println("EEPROM borrada exitosamente.");
  } else {
    Serial.println("Error al borrar la EEPROM.");
  }
}
