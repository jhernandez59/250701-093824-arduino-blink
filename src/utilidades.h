// ------------------------------
// Archivo: utilidades.h
// ------------------------------
#ifndef UTILS_H
#define UTILS_H

#include "eeprom.h"    // <---  para borrar la EEPROM
#include "red_wifi.h"  // <--- para acceder a 'wm'

void informacionSistema();
void gestionarComandosSerie();
String formatearTimestamp(unsigned long timestamp, int offset_horas = -5);

#endif