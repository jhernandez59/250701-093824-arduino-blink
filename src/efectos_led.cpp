// ------------------------------
// Archivo: efectos_led.cpp
// ------------------------------
#include "efectos_led.h"
#include <Arduino.h>

void efectoHeartbeat() {
  static unsigned long ultimoLatido = 0;
  static byte fase = 0;
  const unsigned long tiempos[] = {100, 50, 100,
                                   850};  // ON, OFF, ON, pausa larga

  if (millis() - ultimoLatido >= tiempos[fase]) {
    ultimoLatido = millis();
    digitalWrite(LED_BUILTIN, (fase % 2 == 0) ? LED_ON : LED_OFF);
    fase = (fase + 1) % 4;
  }
}

void efectoMorseOK() {
  static const char codigoMorse[] =
      "1110 1010";  // 1=ON, 0=OFF, espacio=separación
  static byte pos = 0;
  static unsigned long ultimoCambio = 0;

  if (millis() - ultimoCambio >= (codigoMorse[pos] == '1' ? 300 : 100)) {
    ultimoCambio = millis();
    digitalWrite(LED_BUILTIN, codigoMorse[pos] != ' '
                                  ? (codigoMorse[pos] == '1' ? LED_ON : LED_OFF)
                                  : LED_OFF);
    pos = (pos + 1) % strlen(codigoMorse);
  }
}

void efectoBarrido() {
  static int direccion = 1;
  static byte intensidad = 0;
  static unsigned long ultimoCambio = 0;

  if (millis() - ultimoCambio >= 30) {  // Ajusta velocidad
    ultimoCambio = millis();
    intensidad += direccion * 5;
    if (intensidad >= 255 || intensidad <= 0)
      direccion *= -1;
    analogWrite(LED_BUILTIN, intensidad);
  }
}

void efectoEstrella() {
  static unsigned long proximoCambio = 0;

  if (millis() >= proximoCambio) {
    digitalWrite(LED_BUILTIN, random(2) ? LED_ON : LED_OFF);
    proximoCambio = millis() + random(50, 500);  // Aleatoriedad controlada
  }
}

void efectoAlien() {
  static unsigned long ultimoCambio = 0;
  static byte mascara = 0b00000001;

  if (millis() - ultimoCambio >= 100) {
    ultimoCambio = millis();
    digitalWrite(LED_BUILTIN, (mascara & 0b1) ? LED_ON : LED_OFF);
    mascara = (mascara << 1) | (mascara >> 7);  // Rotación de bits
  }
}

void efectoConfirmacion() {
  static unsigned long ultimoCiclo = 0;
  static byte subEstado = 0;

  switch (subEstado) {
    case 0:  // Espera 2 segundos
      if (millis() - ultimoCiclo >= 2000) {
        digitalWrite(LED_BUILTIN, LED_ON);
        ultimoCiclo = millis();
        subEstado = 1;
      }
      break;
    case 1:  // Flash corto
      if (millis() - ultimoCiclo >= 100) {
        digitalWrite(LED_BUILTIN, LED_OFF);
        subEstado = 2;
      }
      break;
    case 2:  // Segundo flash
      if (millis() - ultimoCiclo >= 200) {
        digitalWrite(LED_BUILTIN, LED_ON);
        subEstado = 3;
      }
      break;
    case 3:
      if (millis() - ultimoCiclo >= 300) {
        digitalWrite(LED_BUILTIN, LED_OFF);
        ultimoCiclo = millis();
        subEstado = 0;
      }
      break;
  }
}

void efectoNexus() {
  static unsigned long ultimoCambio = 0;
  static byte patron[] = {150, 100, 80,
                          100, 150, 200};  // Patrón personalizable
  static byte paso = 0;

  if (millis() - ultimoCambio >= patron[paso]) {
    ultimoCambio = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    paso = (paso + 1) % sizeof(patron);
  }
}

// Para el parpadeo de "Enviando Datos"
void parpadeoEnvio() {
  // const int LED_ON = LOW;
  // const int LED_OFF = HIGH;
  digitalWrite(LED_BUILTIN, LED_ON);
  delay(50);
  digitalWrite(LED_BUILTIN, LED_OFF);
  delay(50);
  digitalWrite(LED_BUILTIN, LED_ON);
  delay(50);
  digitalWrite(LED_BUILTIN, LED_OFF);
}