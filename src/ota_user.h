#ifndef OTA_USER_H
#define OTA_USER_H

#include <ESP8266WebServer.h>

// Declaraciones de las funciones públicas del módulo.
void setupOtaUser(ESP8266WebServer& server);
void loopOtaUser();  // Aunque en este caso estará vacío, es buena práctica
                     // tenerlo.

#endif