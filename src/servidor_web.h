// ---------------------------------
// Archivo: servidor_web.h
// ---------------------------------
#ifndef SERVIDOR_WEB_H
#define SERVIDOR_WEB_H

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include "eeprom.h"
#include "red_wifi.h"
#include "sensores.h"
#include "utilidades.h"

extern ESP8266WebServer server;

extern char customNombre[SIZE_NOMBRE];
extern char customLatStr[SIZE_LAT_LON];
extern char customLonStr[SIZE_LAT_LON];
extern char customAltStr[SIZE_ALTURA];

extern float customLatitud;
extern float customLongitud;
extern float customAltura;

void configurarWebServer();
void inicializarMDNS();
#endif