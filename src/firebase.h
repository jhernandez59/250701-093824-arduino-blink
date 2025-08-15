// ------------------------------
// Archivo: firebase.h
// ------------------------------
#ifndef FIREBASE_H
#define FIREBASE_H

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "eeprom.h"
#include "red_wifi.h"
#include "secrets.h"  // 🆕 Incluye claves y configuración sensible
#include "sensores.h"
#include "utilidades.h"

extern char customNombre[SIZE_NOMBRE];
extern char customLatStr[SIZE_LAT_LON];
extern char customLonStr[SIZE_LAT_LON];
extern char customAltStr[SIZE_ALTURA];

extern float customLatitud;
extern float customLongitud;
extern float customAltura;

void configurarFirebase();
void registrarDatosSensores();
bool permisoFirebase(String);
int getContadorDesconexiones();
void registrarPresionHistorial();

#endif
