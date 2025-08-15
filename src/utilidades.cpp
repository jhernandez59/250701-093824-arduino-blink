// ------------------------------
// Archivo: utilidades.cpp
// ------------------------------
#include "utilidades.h"
#include <ESP8266WiFi.h>
#include "estado.h"

void informacionSistema() {
  Serial.println("\n📟 Información del sistema:");
  Serial.printf("Estado: %s\n", estadoDispositivoATexto(estadoActual));
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  Serial.printf("OTA para Usuario listo. Accede a http://%s/update\n",
                WiFi.localIP().toString().c_str());
  Serial.println();
}

void gestionarComandosSerie() {
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "reset") {
      Serial.println(
          "🔁 RESET TOTAL: Borrando credenciales WiFi y todos los parámetros "
          "personalizados...");

      // 1. Borra las credenciales WiFi que gestiona WiFiManager
      wm.resetSettings();

      // 2. Borra tus parámetros personalizados de la EEPROM
      clearConfigEEPROM();

      Serial.println(
          "¡Borrado completo! Reiniciando para entrar en modo portal.");
      delay(1000);
      ESP.restart();
    }

    if (comando == "clear") {
      Serial.println(
          "🧹 CLEAR: Borrando solo los parámetros personalizados (nombre, lat, "
          "lon...).");
      Serial.println("Las credenciales WiFi se conservarán.");

      // Borra solo tus parámetros personalizados de la EEPROM
      clearConfigEEPROM();

      Serial.println(
          "¡Parámetros borrados! Reiniciando para recargar valores por "
          "defecto.");
      delay(1000);
      ESP.restart();
    }

    if (comando == "info") {
      informacionSistema();
    }
  }
}

String ceros(int numero) {
  return (numero < 10 ? "0" : "") + String(numero);
}

String formatearTimestamp(unsigned long timestamp, int offset_horas) {
  // Ajuste por zona horaria
  timestamp += offset_horas * 3600;

  int seg = timestamp % 60;
  timestamp /= 60;
  int min = timestamp % 60;
  timestamp /= 60;
  int hora = timestamp % 24;
  timestamp /= 24;

  // Convertir días desde 1970 a fecha calendario
  int z = timestamp + 719468;
  int era = (z >= 0 ? z : z - 146096) / 146097;
  int doe = z - era * 146097;
  int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int y = yoe + era * 400;
  int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  int mp = (5 * doy + 2) / 153;
  int d = doy - (153 * mp + 2) / 5 + 1;
  int m = mp + (mp < 10 ? 3 : -9);
  y = y + (m <= 2);

  return String(y) + "-" + ceros(m) + "-" + ceros(d) + " " + ceros(hora) + ":" +
         ceros(min) + ":" + ceros(seg);
}
