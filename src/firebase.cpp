// ------------------------------
// Archivo: firebase.cpp
// ------------------------------
#include "firebase.h"
#include "estado.h"
#include "secrets.h"
#include "version.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig conf;

void configurarFirebase() {
  conf.database_url = FIREBASE_HOST;
  conf.signer.tokens.legacy_token = FIREBASE_KEY;
  Firebase.begin(&conf, &auth);
  Firebase.reconnectWiFi(true);
}

bool permisoFirebase(String mac) {
  // --- LÍNEA DE DEPURACIÓN CLAVE ---
  Serial.print("Intentando verificar permiso para la MAC: [");
  Serial.print(mac);
  Serial.println("]");

  String path = "/macs_permitidas/" + mac;
  Serial.println("Consultando la ruta en Firebase: " + path);

  if (Firebase.RTDB.getBool(&fbdo, path)) {
    if (fbdo.boolData()) {
      Serial.println("✅ MAC permitida. Puedes enviar datos.");
      return true;
    } else {
      Serial.println(
          "⛔ La consulta a Firebase tuvo éxito, pero el valor encontrado no "
          "es 'true'.");
      return false;
    }
  } else {
    Serial.print("❌ Error al ejecutar la consulta getBool: ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

void registrarDatosSensores() {
  if (!Firebase.ready()) {
    Serial.println("Firebase no está listo, esperando...");
    return;
  }

  String macAddress = obtenerMacAddress();

  if (!permisoFirebase(macAddress)) {
    return;
  }

  DatosSensores datosNuevos = leerSensores();
  if (!datosNuevos.ahtValido && !datosNuevos.bmpValido) {
    Serial.println("Lectura de sensores no válida.");
    return;
  }

  estadoAnterior = estadoActual;
  estadoActual = ENVIANDO_DATOS;

  // --- OBTENER LECTURA ANTERIOR ---
  String path = "/sensores_en_tiempo_real/" + macAddress;
  float t_anterior = 0, h_anterior = 0, p_anterior = 0;
  // Usamos double para el timestamp por su gran tamaño.
  double ts_anterior = 0;

  // Intentamos obtener los valores anteriores de la ruta "actual"
  // Es importante comprobar el valor de retorno de getFloat para la primera
  // ejecución, cuando no hay datos.
  if (Firebase.RTDB.getFloat(&fbdo, path + "/actual/temperatura")) {
    t_anterior = fbdo.floatData();
  }
  if (Firebase.RTDB.getFloat(&fbdo, path + "/actual/humedad")) {
    h_anterior = fbdo.floatData();
  }
  if (Firebase.RTDB.getFloat(&fbdo, path + "/actual/presion")) {
    p_anterior = fbdo.floatData();
  }

  // --- ¡¡LECTURA DEL TIMESTAMP ANTERIOR!! ---
  if (Firebase.RTDB.getDouble(&fbdo, path + "/actual/timestamp")) {
    ts_anterior = fbdo.doubleData();  // Guardamos el número del timestamp
    Serial.print("Timestamp anterior recuperado: ");
    Serial.println(String(ts_anterior, 0));
  }

  // --- CONSTRUIR UN ÚNICO OBJETO JSON ---
  FirebaseJson rootJson;

  // Datos de ubicación y nombre (en la raíz del dispositivo)
  rootJson.set("nombre", customNombre);
  rootJson.set("latitud", customLatitud);
  rootJson.set("longitud", customLongitud);
  rootJson.set("altura", customAltura);

  // --- Sub-nodo "anterior" ---
  rootJson.set("anterior/temperatura", t_anterior);
  rootJson.set("anterior/humedad", h_anterior);
  rootJson.set("anterior/presion", p_anterior);
  // --- ¡CAMBIO CLAVE 1! ---
  // Escribimos el NÚMERO que leímos, no la instrucción ".sv".
  // Para que Firebase lo interprete como número, es mejor usar set(...,
  // (double)ts_anterior) o asegurarse de que la variable es double.
  rootJson.set("anterior/timestamp", ts_anterior);

  // --- Sub-nodo "actual" (con los datos nuevos) ---
  rootJson.set("actual/temperatura", datosNuevos.temperatura);
  rootJson.set("actual/humedad", datosNuevos.humedad);
  rootJson.set("actual/presion", datosNuevos.presion);
  rootJson.set("actual/timestamp/.sv",
               "timestamp");  // Marca de tiempo de la nueva lectura

  // --- Sub-nodo "diferencia" ---
  rootJson.set("diferencia/temperatura", datosNuevos.temperatura - t_anterior);
  rootJson.set("diferencia/humedad", datosNuevos.humedad - h_anterior);
  rootJson.set("diferencia/presion", datosNuevos.presion - p_anterior);

  // --- Sub-nodo "estado" ---
  int desconexiones = getContadorDesconexiones();
  rootJson.set("estado/aht20", datosNuevos.ahtValido ? "OK" : "FALLA");
  rootJson.set("estado/bmp280", datosNuevos.bmpValido ? "OK" : "FALLA");
  rootJson.set("estado/ip", WiFi.localIP().toString());
  rootJson.set("estado/ssid", WiFi.SSID());
  rootJson.set("estado/rssi", WiFi.RSSI());
  rootJson.set("estado/reintentos_wifi", desconexiones);
  rootJson.set("estado/tiempo_on", millis() / 1000);

  // --- Sub-nodo "version" ---
  rootJson.set("version/numero", String(APP_VERSION));
  rootJson.set("version/clave", String(APP_NOMBRE_CLAVE));
  rootJson.set("version/proyecto", String(APP_NOMBRE_PROYECTO));

  // --- Marca de tiempo principal ---
  rootJson.set("ultimo_reporte/.sv", "timestamp");

  // --- HACER UNA ÚNICA ESCRITURA ---
  Serial.println("Enviando JSON completo a Firebase...");
  if (Firebase.RTDB.setJSON(&fbdo, path, &rootJson)) {
    Serial.println("✅ Datos completos enviados correctamente.");
  } else {
    Serial.print("❌ Error al enviar JSON completo: ");
    Serial.println(fbdo.errorReason());
  }

  estadoActual = estadoAnterior;  // Volver al estado anterior
}