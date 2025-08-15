// ------------------------------
// Archivo: wifi_manager.cpp
// ------------------------------
#include "red_wifi.h"
#include "credenciales.h"
#include "eeprom.h"
#include "estado.h"

#include <ESP8266WiFi.h>
#include <WiFiManager.h>

// --- Variables del dispositivo
char customNombre[SIZE_NOMBRE] = "El Insurgente del Sur Global";
char customLatStr[SIZE_LAT_LON] = "4.55242";    // La Abadia@Armenia
char customLonStr[SIZE_LAT_LON] = "-75.66576";  // La Abadia@Armenia
char customAltStr[SIZE_ALTURA] = "1480";  // Altura Armenia (Bogota 2640 m)

float customLatitud = 4.55242;
float customLongitud = -75.66576;
float customAltura = 1480;

ConfigWM config;  // variables del dispositivo guardados en EEPROM

WiFiManager wm;
WiFiManagerParameter* param_nombre;
WiFiManagerParameter* param_lat;
WiFiManagerParameter* param_lon;
WiFiManagerParameter* param_alt;

const char* nombrePortal = "Sensor Ambiente";
bool guardarConfig = false;  // flag para guardar en EEPROM callback WiFiManager
volatile int contadorDesconexiones =
    0;  // Nuestro contador de problemas de conexión

// Constantes para la configuracion
const int PORTAL_TIMEOUT_S = 60;  // Timeout del portal en segundos
const int MAX_INTENTOS_RECONEXION = 5;
const int DELAY_REINTENTO_MS = 3000;

// Configurar conexion a red WiFi
void configurar_wifi() {
  //----------------------------
  // 1. Configura WiFiManager
  //----------------------------
  // Configura un timeout razonable (ej. 1 minuto)
  wm.setTimeout(PORTAL_TIMEOUT_S);  // En segundos.

  // Asignamos nuestra función de callback. Se llamará cuando el
  // usuario guarde los datos de configuracion la usamos para guardar
  // los datos en EEPORM
  wm.setSaveConfigCallback(guardarConfigCallback);

  //---------------------------------------------
  // Copiar los datos almacenados en el EEPORM
  //---------------------------------------------
  loadConfigEEPROM(config);

  strncpy(customNombre, config.nombre, SIZE_NOMBRE);
  strncpy(customLatStr, config.lat, SIZE_LAT_LON);
  strncpy(customLonStr, config.lon, SIZE_LAT_LON);
  strncpy(customAltStr, config.alt, SIZE_ALTURA);

  customNombre[SIZE_NOMBRE - 1] = '\0';
  customLatStr[SIZE_LAT_LON - 1] = '\0';
  customLonStr[SIZE_LAT_LON - 1] = '\0';
  customAltStr[SIZE_ALTURA - 1] = '\0';

  customLatitud = atof(customLatStr);
  customLongitud = atof(customLonStr);
  customAltura = atof(customAltStr);

  // 2. Crea campos personalizados en el portal
  param_nombre =
      new WiFiManagerParameter("nombre", "Nombre-Ubicacion (max: 50 carateres)",
                               customNombre, SIZE_NOMBRE);

  param_lat = new WiFiManagerParameter("latitud", "Latitud (ej: 4.7110)",
                                       customLatStr, SIZE_LAT_LON);
  param_lon = new WiFiManagerParameter("longitud", "Longitud (ej: -74.0721)",
                                       customLonStr, SIZE_LAT_LON);
  param_alt = new WiFiManagerParameter("altura", "Altura (ej: 2640)",
                                       customAltStr, SIZE_ALTURA);

  // 3. Añade los parámetros al portal
  wm.addParameter(param_nombre);
  wm.addParameter(param_lat);
  wm.addParameter(param_lon);
  wm.addParameter(param_alt);

  // Intenta reconectarse antes de abrir el portal de configuracion
  int intentos = 0;
  bool conectado = false;

  // Si ya hay credenciales WiFi guardadas, evitar abrir el portal
  if (WiFi.SSID() != "") {
    Serial.println("Ya hay red WiFi configurada. Intentando reconectar...");
    wm.setEnableConfigPortal(
        false);  // No queremos que autoConnect abra el portal aún

    while (intentos < MAX_INTENTOS_RECONEXION) {
      // autoConnect devuelve true si se conecta. El timeout interno de
      // autoConnect es lo que determina cuánto espera en cada intento.
      if (wm.autoConnect()) {
        conectado = true;
        break;
      }
      intentos++;
      Serial.printf("Intento %d/%d fallido. Reintentando en 3 segundos...\n",
                    intentos, MAX_INTENTOS_RECONEXION);
      delay(DELAY_REINTENTO_MS);
    }
  }

  // Si después de todo, no estamos conectados (ya sea porque no había red
  // guardada o porque los reintentos fallaron), AHORA SÍ lanzamos el portal.
  if (!conectado) {
    Serial.println("No se pudo conectar. Abriendo portal de configuración...");
    // Ahora sí, lanza el portal que se quedará esperando
    wm.setEnableConfigPortal(true);  // Permitimos que se abra el portal

    // Lanzamos el portal con un timeout. Si el usuario no hace nada,
    // continuará.
    if (wm.autoConnect(nombrePortal)) {
      conectado = true;  // Se conectó a través del portal
    }
  }

  // Verificación final. Si ni los reintentos ni el portal funcionaron...
  if (!conectado) {
    Serial.println("Falló la conexión. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  if (guardarConfig) {
    // 7. Guarda en la EEPROM (persistencia)
    strncpy(config.nombre, customNombre, SIZE_NOMBRE);
    strncpy(config.lat, customLatStr, SIZE_LAT_LON);
    strncpy(config.lon, customLonStr, SIZE_LAT_LON);
    strncpy(config.alt, customAltStr, SIZE_ALTURA);

    saveConfigEEPROM(config);
    // Debug
    Serial.printf(
        "Configuración guardada: %s (Lat: %.5f, Lon: %.5f Altura: %.5f) \n",
        customNombre, customLatitud, customLongitud, customAltura);

    // Es buena práctica resetear el flag después de usarlo
    guardarConfig = false;
  }
}

void guardarConfigCallback() {
  Serial.println("[CALLBACK] Validando y preparando nuevos parámetros...");
  bool datosValidos = true;  // Un flag para rastrear si todo es correcto

  // 1. Obtener los valores del portal como Strings de Arduino para facilitar la
  // manipulación.
  String nombreStr_nuevo = param_nombre->getValue();
  String latStr_nuevo = param_lat->getValue();
  String lonStr_nuevo = param_lon->getValue();
  String altStr_nuevo = param_alt->getValue();

  // --- Validación del Nombre ---
  // strncpy es perfecto aquí.
  strncpy(customNombre, nombreStr_nuevo.c_str(), SIZE_NOMBRE);
  customNombre[SIZE_NOMBRE - 1] = '\0';  // Asegurar terminación nula

  // --- Validación de Coordenadas ---
  // Reemplazamos comas por puntos para ser más flexibles con la entrada del
  // usuario.
  latStr_nuevo.replace(',', '.');
  lonStr_nuevo.replace(',', '.');

  float lat_temp = latStr_nuevo.toFloat();  // toFloat es más robusto que atof
  float lon_temp = lonStr_nuevo.toFloat();

  if (lat_temp >= -90.0 && lat_temp <= 90.0 && lon_temp >= -180.0 &&
      lon_temp <= 180.0) {
    // Si la validación PASA, actualizamos nuestras variables principales.
    customLatitud = lat_temp;
    customLongitud = lon_temp;

    // Y también actualizamos las cadenas de texto por consistencia.
    strncpy(customLatStr, latStr_nuevo.c_str(), SIZE_LAT_LON);
    strncpy(customLonStr, lonStr_nuevo.c_str(), SIZE_LAT_LON);
    customLatStr[SIZE_LAT_LON - 1] = '\0';
    customLonStr[SIZE_LAT_LON - 1] = '\0';
    Serial.println("✅ Coordenadas validadas y actualizadas.");
  } else {
    // OJO: Si la validación FALLA, no hacemos nada. Mantenemos los valores
    // anteriores.
    Serial.println(
        "❌ Coordenadas inválidas. Se mantendrán los valores anteriores.");
    datosValidos = false;  // Marcamos que algo salió mal
  }

  // --- Validación de Altura ---
  altStr_nuevo.replace(',', '.');
  float alt_temp = altStr_nuevo.toFloat();

  if (alt_temp >= -450.0 &&
      alt_temp <= 8848.0) {  // Rango desde el Mar Muerto hasta el Everest
    // Si la validación PASA, actualizamos la variable principal.
    customAltura = alt_temp;
    strncpy(customAltStr, altStr_nuevo.c_str(), SIZE_ALTURA);
    customAltStr[SIZE_ALTURA - 1] = '\0';
    Serial.println("✅ Altura validada y actualizada.");
  } else {
    // Si la validación FALLA, mantenemos el valor anterior.
    Serial.println("❌ Altura inválida. Se mantendrá el valor anterior.");
    datosValidos = false;  // Marcamos que algo salió mal
  }

  // Solo activamos el guardado si TODOS los datos fueron válidos
  if (datosValidos) {
    Serial.println("Todos los parámetros son válidos. Se procederá a guardar.");
    guardarConfig = true;
  }
}

void configurarEventosWiFi() {
  static WiFiEventHandler onGotIP, onDisconnected, onConnected;

  onGotIP = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
    Serial.printf("Conectado! IP: %s\n", event.ip.toString().c_str());
    sistemaEstado = READY;  // ¡Sistema listo!
    estadoActual = OPERACION_NORMAL;
    // Debug
    // Iniciar servicios dependientes del WiFi aquí
    // iniciarServiciosWeb();
  });

  onDisconnected = WiFi.onStationModeDisconnected(
      [](const WiFiEventStationModeDisconnected& event) {
        Serial.printf("Desconectado! Razón: %d\n", event.reason);
        sistemaEstado =
            (wm.getConfigPortalActive()) ? WIFI_CONFIG : WIFI_CONNECTING;
        estadoActual = ERROR_CRITICO;
        Serial.printf("Evento Disconnected capturado, nuevo estado: %d\n",
                      sistemaEstado);
      });

  onConnected = WiFi.onStationModeConnected(
      [](const WiFiEventStationModeConnected& event) {
        sistemaEstado = WIFI_CONNECTING;
        estadoActual = CONECTANDO_WIFI;
        gestionarLedDeEstado();

        // Debug adicional
        Serial.println(
            "Evento Connected capturado, cambiando a estado WIFI_CONNECTING");
      });
}

// Puedes crear una función para obtener el valor de forma segura
int getContadorDesconexiones() {
  return contadorDesconexiones;
}

String obtenerMacAddress() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  return mac;
}