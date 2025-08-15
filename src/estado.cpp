// ------------------------------
// Archivo: estado.cpp
// ------------------------------
#include "estado.h"

SistemaEstado sistemaEstado = BOOT;
EstadoDispositivo estadoActual = INICIANDO;
EstadoDispositivo estadoAnterior = INICIANDO;

// 🟢 Para controlar el parpadeo rápido durante ENVIANDO_DATOS
static unsigned long inicioParpadeoEnvio = 0;
static int etapaParpadeo = 0;
static bool enParpadeoEnvio = false;

// const int pinLed = LED_BUILTIN;

void gestionarLedDeEstado() {
  static unsigned long ultimoCambioLed = 0;
  unsigned long ahora = millis();

  // La lógica de encendido/apagado depende de la placa.
  // LOW = ON para muchas placas ESP. ¡Ajusta si es necesario!
  // const int LED_ON = LOW;
  // const int LED_OFF = HIGH;

  switch (estadoActual) {
    case INICIANDO:
      digitalWrite(LED_PIN, LED_ON);  // LED sólido encendido
      break;

    case CONECTANDO_WIFI:  // Parpadeo medio: 500ms ON, 500ms OFF
      if (ahora - ultimoCambioLed >= 500) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Invierte el estado
        ultimoCambioLed = ahora;
      }
      break;

    case ERROR_CRITICO:  // Parpadeo rápido: 100ms ON, 100ms OFF
      if (ahora - ultimoCambioLed >= 100) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        ultimoCambioLed = ahora;
      }
      break;

    case OPERACION_NORMAL:  // "Latido": 100ms ON, 2900ms OFF
      // Esta lógica es un poco más compleja para un patrón asimétrico
      if (digitalRead(LED_PIN) == LED_ON) {  // Si el LED está encendido...
        if (ahora - ultimoCambioLed >=
            100) {  // ...lo apagamos después de 100ms
          digitalWrite(LED_PIN, LED_OFF);
          ultimoCambioLed = ahora;
        }
      } else {  // Si el LED está apagado...
        if (ahora - ultimoCambioLed >=
            2900) {  // ...lo encendemos después de 2.9s
          digitalWrite(LED_PIN, LED_ON);
          ultimoCambioLed = ahora;
        }
      }
      break;

    // Los casos ENVIANDO_DATOS y MODO_OTA los manejaremos de forma especial.
    // Por ahora, en esos casos no hacemos nada aquí para no interferir.
    case ENVIANDO_DATOS: {
      const unsigned long INTERVALO = 50;

      if (!enParpadeoEnvio) {
        // Iniciar parpadeo
        inicioParpadeoEnvio = ahora;
        etapaParpadeo = 0;
        enParpadeoEnvio = true;
      }

      if (enParpadeoEnvio && ahora - inicioParpadeoEnvio >= INTERVALO) {
        inicioParpadeoEnvio = ahora;

        // Alternar LED cada etapa
        if (etapaParpadeo % 2 == 0) {
          digitalWrite(LED_PIN, LED_ON);  // 0, 2, 4 ... 10
        } else {
          digitalWrite(LED_PIN, LED_OFF);  // 1, 3, 5 ... 9
        }

        etapaParpadeo++;

        // 🔁 Cuando se completan 10 etapas (5 ciclos ON/OFF)
        if (etapaParpadeo >= 10) {
          enParpadeoEnvio = false;
          estadoActual = estadoAnterior;  // ✅ Volver al estado anterior
        }
      }
      break;
    }
    case MODO_OTA:
      if (ahora - ultimoCambioLed >= 125) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        ultimoCambioLed = ahora;
      }
      break;

    default:
      // Apagar el LED si el estado es desconocido o se maneja en otro lado
      digitalWrite(LED_PIN, LED_OFF);
      break;
  }
}

const char* estadoSistemaATexto(SistemaEstado estado) {
  switch (estado) {
    case BOOT:
      return "BOOT";
    case WIFI_CONNECTING:
      return "WIFI_CONNECTING";
    case WIFI_CONFIG:
      return "WIFI_CONFIG";
    case READY:
      return "READY";
    case ERROR:
      return "ERROR";
    default:
      return "DESCONOCIDO";
  }
}

const char* estadoDispositivoATexto(EstadoDispositivo estado) {
  switch (estado) {
    case INICIANDO:
      return "INICIANDO";
    case CONECTANDO_WIFI:
      return "CONECTANDO_WIFI";
    case ERROR_CRITICO:
      return "ERROR_CRITICO";
    case OPERACION_NORMAL:
      return "OPERACION_NORMAL";
    case ENVIANDO_DATOS:
      return "ENVIANDO_DATOS";
    case MODO_OTA:
      return "MODO_OTA";
    default:
      return "DESCONOCIDO";
  }
}
