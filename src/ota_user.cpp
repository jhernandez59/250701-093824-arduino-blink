#include "ota_user.h"
#include <Arduino.h>
#include <ESP8266HTTPUpdateServer.h>

// Creamos los objetos necesarios para el actualizador web.
// Son 'static' para que solo sean visibles dentro de este archivo.
static ESP8266HTTPUpdateServer httpUpdater;

// Esta función configura el servidor web para que tenga la página de
// actualización. Le pasamos el objeto 'server' desde main.cpp para no crear dos
// servidores.
void setupOtaUser(ESP8266WebServer& server) {
  Serial.println("Inicializando OTA para Usuario (Web Updater)...");

  // El objeto httpUpdater necesita una referencia al servidor web principal.
  // El segundo argumento es la ruta (URL) donde estará la página de
  // actualización. El tercer y cuarto argumento son para la autenticación
  // (usuario y contraseña).
  httpUpdater.setup(&server, "/update", "admin", "admin");

  // Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  // Serial.println("OTA para Usuario listo. Accede a http://<IP>/update");
  Serial.printf("OTA para Usuario listo. Accede a http://%s/update\n",
                WiFi.localIP().toString().c_str());
  Serial.println("Usuario: admin, Contraseña: admin");
}

// En este diseño simple, el servidor web se maneja en el loop principal,
// así que esta función no necesita hacer nada.
void loopOtaUser() {
  // Nada que hacer aquí, ya que server.handleClient() se llama en main.
}