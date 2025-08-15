#include "ota_dev.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "estado.h"

// Esta función configura y arranca el servicio ArduinoOTA.
void setupOtaDev() {
  Serial.println("Inicializando OTA para Desarrollador (ArduinoOTA)...");

  // Opcional: Nombre del dispositivo en la red.
  ArduinoOTA.setHostname("esp8266-test");

  // Opcional: Contraseña para las actualizaciones.
  // ArduinoOTA.setPassword("mi_clave_dev");

  ArduinoOTA.onStart([]() {
    estadoAnterior = estadoActual;
    estadoActual = MODO_OTA;

    Serial.println("Inicio de actualización OTA (Desarrollador)...");
  });

  ArduinoOTA.onEnd([]() {
    estadoActual = estadoAnterior;
    Serial.println("\nFin de la actualización.");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progreso: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error de OTA [%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Fallo de autenticación");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Fallo al iniciar");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Fallo de conexión");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Fallo al recibir");
    else if (error == OTA_END_ERROR)
      Serial.println("Fallo al finalizar");
  });

  ArduinoOTA.begin();
  Serial.println("OTA para Desarrollador listo.");
}

// Esta función debe ser llamada en cada ciclo del loop principal.
void loopOtaDev() {
  ArduinoOTA.handle();
}