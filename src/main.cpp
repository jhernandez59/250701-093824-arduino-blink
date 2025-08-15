// ------------------------------
// Archivo: main.cpp
// ------------------------------
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

// Incluimos nuestros módulos y las credenciales
#include "credenciales.h"
#include "efectos_led.h"
#include "estado.h"
#include "firebase.h"
#include "ota_dev.h"
#include "ota_user.h"
#include "red_wifi.h"
#include "sensores.h"
#include "servidor_web.h"
#include "utilidades.h"
#include "version.h"

// Creamos el objeto del servidor web. Será usado por el OTA de usuario.
ESP8266WebServer server(80);

// Variables para el envío de datos
const unsigned long INTERVALO_NORMAL_ENVIO = 15 * 60 * 1000;  // 15 minutos
static unsigned long ultimoEnvio = 0;

const unsigned long PRIMER_INTERVALO_ENVIO = 2 * 60 * 1000;  // 2 minutos
bool primerEnvioPendiente = true;  // Nuestro nuevo flag

// Variables para la verificación de permisos
const unsigned long INTERVALO_VERIFICACION = 60 * 60 * 1000;  // 1 hora
static unsigned long ultimaVerificacionPermiso = 0;

// >>> NUEVAS VARIABLES PARA EL HISTORIAL DE PRESIÓN
unsigned long ultimaEscrituraHistorial = 0;
const unsigned long INTERVALO_HISTORIAL_PRESION =
    3600000UL;  // 1 hora en milisegundos (60 * 60 * 1000)

// Bandera global para controlar el permiso
bool permiso_firebase_OK = false;

String macAddress;  // Para guardar la MAC y no obtenerla todo el tiempo

void setup() {
  // -------------------------------------------------
  // --- Inicializa hardware basico, aviso de inicio
  // -------------------------------------------------
  estadoActual = INICIANDO;
  pinMode(LED_BUILTIN, OUTPUT);
  gestionarLedDeEstado();  // Para que el LED se encienda inmediatamente

  // ... Iniciar Serial ...
  Serial.begin(115200);
  while (!Serial && millis() < 1000)
    ;  // Espera hasta 1 segundo por Serial
  Serial.println("\n\n--- Monitor Ambiental Iniciando  ---");
  Serial.print("Versión del Firmware: ");
  // Serial.println(FIRMWARE_VERSION);
  Serial.println(APP_VERSION);

  // ---------- Conexión Wi-Fi ----------
  estadoActual = CONECTANDO_WIFI;
  configurarEventosWiFi();
  configurar_wifi();

  // ---------- Inicializa lo Sensores ----------
  inicializarSensores();

  // ---------- Configurar Servidor Web ----------
  configurarWebServer();

  // ---------- Configurar Firebase ----------
  configurarFirebase();

  // --- PRIMERA VERIFICACIÓN DE PERMISO ---
  // Se ejecuta solo una vez al arrancar para saber el estado inicial.
  Serial.println("Verificando permiso inicial de Firebase...");
  macAddress = obtenerMacAddress();
  if (permisoFirebase(macAddress)) {
    permiso_firebase_OK = true;
  } else {
    permiso_firebase_OK = false;
  }
  // Inicializamos el temporizador de verificación para que no se ejecute
  // inmediatamente en el primer loop.
  ultimaVerificacionPermiso = millis();

  // ---------- Configuración de OTA ----------
  // --- Configuración de Módulos ---
  // Llama a la configuración de cada módulo
  setupOtaDev();         // Inicia el OTA para desarrollador
  setupOtaUser(server);  // Inicia el OTA para usuario, pasándole el servidor

  // Inicia el servidor web después de configurar todas sus rutas
  server.begin();
  Serial.println("Servidor HTTP iniciado.");

  // OJO: Inciar mDNS despues de iniciar el Web Server
  inicializarMDNS();

  // ---------- Configuración de Watchdog Timer ----------
  Serial.println("Activando Watchdog Timer (8 segundos)...");
  // Habilita el Watchdog con un timeout de 8 segundos.
  // Si el loop() se congela por más tiempo, el ESP8266 se reiniciará.
  ESP.wdtEnable(8000);

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(
      WiFi.localIP());  // Imprime la IP, ¡la necesitarás para subir el código!
}

void loop() {
  // --- ¡PRIMERA LÍNEA DE TU LOOP! ---
  // "Alimenta" al perro guardián para decirle que el código sigue vivo.
  ESP.wdtFeed();

  // --- para los controles del tiempo
  unsigned long ahora = millis();

  // --- Manejo de Módulos en el Loop ---
  // Llama a la función de manejo de cada módulo
  loopOtaDev();  // Revisa si hay actualizaciones de desarrollador

  MDNS.update();  // Necesario para mantener mDNS activo

  server.handleClient();  // Revisa si hay peticiones en el servidor web (para
                          // el OTA de usuario)

  // ¡Llama a esta función en cada ciclo!
  gestionarLedDeEstado();

  // --- Lógica Principal ---
  // ----------------------------------------------
  // --- Lectura datos sensores y envio a Firebase
  // ----------------------------------------------
  // --- BLOQUE 1: VERIFICACIÓN PERIÓDICA DE PERMISOS (CADA HORA) ---
  // Este bloque es independiente del envío de datos. Su única misión
  // es actualizar la variable 'permiso_firebase_OK' de vez en cuando.
  if (ahora - ultimaVerificacionPermiso >= INTERVALO_VERIFICACION) {
    Serial.println("------------------------------------");
    Serial.println("Ha pasado 1 hora. Re-verificando permiso en Firebase...");

    if (permisoFirebase(macAddress)) {
      permiso_firebase_OK = true;
    } else {
      permiso_firebase_OK = false;
    }

    // Actualizamos el temporizador de verificación para la próxima hora.
    ultimaVerificacionPermiso = ahora;
    Serial.println("------------------------------------");
  }

  // --- BLOQUE 2: ENVÍO PERIÓDICO DE DATOS (CADA 5 MINUTOS) ---
  // Este bloque solo mira la bandera 'permiso_firebase_OK' para decidir si
  // envía. No le importa cuándo se actualizó, solo su valor actual.
  // Comprobamos si es momento de enviar
  bool esMomentoDeEnviar = false;
  if (primerEnvioPendiente) {
    // Lógica para el PRIMER envío
    if (ahora - ultimoEnvio >= PRIMER_INTERVALO_ENVIO) {
      esMomentoDeEnviar = true;
      primerEnvioPendiente = false;  // Marcamos que el primer envío ya se hará
    }
  } else {
    // Lógica para los envíos NORMALES
    if (ahora - ultimoEnvio >= INTERVALO_NORMAL_ENVIO) {
      esMomentoDeEnviar = true;
    }
  }

  // Si se cumple CUALQUIERA de las condiciones, enviamos
  if (esMomentoDeEnviar) {
    Serial.print("Intentando enviar datos... ");

    if (permiso_firebase_OK) {
      Serial.println("Permiso OK. Enviando...");
      registrarDatosSensores();  // Tu función que envía los datos
    } else {
      Serial.println("Permiso DENEGADO. No se enviarán datos.");
    }

    // Actualizamos el temporizador de envío para los próximos 5 minutos.
    ultimoEnvio = ahora;
  }

  // >>> --- BLOQUE 3: NUEVO BLOQUE PARA ENVIAR HISTORIAL DE PRESIÓN (CADA HORA)
  // --- Este bloque es completamente independiente y se encarga de registrar la
  // presión una vez por hora para construir la tendencia.
  if (ahora - ultimaEscrituraHistorial >= INTERVALO_HISTORIAL_PRESION) {
    Serial.print("Intentando enviar presión al historial... ");

    // También respetamos el permiso de Firebase para esta operación
    if (permiso_firebase_OK) {
      Serial.println("Permiso OK. Enviando...");
      // Llamamos a una nueva función específica para esta tarea
      registrarPresionHistorial();
    } else {
      Serial.println("Permiso DENEGADO. No se enviará historial de presión.");
    }

    // Actualizamos el temporizador para la próxima hora.
    ultimaEscrituraHistorial = ahora;
  }

  // ... otro código que necesite el loop ...

  gestionarComandosSerie();  // <--- LLAMA A LA FUNCIÓN AQUÍ

  yield();
}
