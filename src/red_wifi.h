// ----------------------
// Archivo: red_wifi.h
// ----------------------
#ifndef RED_WIFI_H
#define RED_WIFI_H

#include <WiFiManager.h>  // <--- Añade este include

extern WiFiManager wm;  // <--- AÑADE ESTA LÍNEA

void configurar_wifi();
void guardarConfigCallback();
void configurarEventosWiFi();
String obtenerMacAddress();

#endif