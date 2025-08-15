// ------------------------------
// Archivo: estado.h
// ------------------------------
#ifndef ESTADO_H
#define ESTADO_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "efectos_led.h"

#define LED_PIN LED_BUILTIN  // Usar el LED integrado

// --- Estados del sistema Red WiFi
enum SistemaEstado { BOOT, WIFI_CONNECTING, WIFI_CONFIG, READY, ERROR };
extern SistemaEstado sistemaEstado;

/*
// --- Estados del Dispositivo ---
//
Estado	            Patrón del LED	            Significado
INICIANDO	        Sólido ENCENDIDO	        "Estoy arrancando,
espera."
CONECTANDO_WIFI	    Parpadeo Medio (1 vez/seg)	"Trabajando para
conectar..."
ERROR_CRITICO	    Parpadeo Rápido y Continuo	"¡AYUDA! ¡Algo
grave pasó!"
OPERACION_NORMAL	"Latido" lento (un pulso corto cada 3 seg) "Todo
bien, estoy vivo y esperando."
ENVIANDO_DATOS	    Doble parpadeo rápido       "¡Enviando datos ahora mismo!"
MODO_OTA	        Parpadeo Rítmico(blink-blink-pausa)
"No me apagues, estoy en cirugía."
*/
// Enum para tener nombres claros para cada estado
enum EstadoDispositivo {
  INICIANDO,
  CONECTANDO_WIFI,
  ERROR_CRITICO,
  OPERACION_NORMAL,
  ENVIANDO_DATOS,
  MODO_OTA
};

// Variable global para almacenar el estado actual del dispositivo
extern EstadoDispositivo estadoActual;
extern EstadoDispositivo estadoAnterior;

void gestionarLedDeEstado();
const char* estadoSistemaATexto(SistemaEstado estado);

const char* estadoDispositivoATexto(EstadoDispositivo estado);

#endif