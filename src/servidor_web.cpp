// ---------------------------------
// Archivo: servidor_web.cpp
// ---------------------------------
#include "servidor_web.h"
#include "version.h"

void configurarWebServer() {
  WiFi.mode(WIFI_STA);  // Importante establecer el modo primero

  server.on("/", HTTP_GET,
            []() {
              int rssi = WiFi.RSSI();
              String calidad;
              String color;

              if (rssi >= -60) {
                calidad = "Fuerte 🟢";
                color = "#2ecc71";
              } else if (rssi >= -75) {
                calidad = "Media 🟡";
                color = "#f1c40f";
              } else {
                calidad = "Débil 🔴";
                color = "#e74c3c";
              }

              String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <!-- AQUÍ LA MAGIA: Actualiza la página cada 10 segundos -->
  <meta http-equiv='refresh' content='10'> 
  <title>Sensor Ambiental</title>
  <style>
    body {
      background-color: #121212;
      color: #ffffff;
      font-family: Arial, sans-serif;
      margin: 20px;
    }
    h1 {
      border-bottom: 1px solid #444;
      padding-bottom: 5px;
    }
    p {
      margin: 6px 0;
    }
    a {
      color: #1e90ff;
      text-decoration: none;
    }
    a:hover {
      text-decoration: underline;
    }
    footer {
      margin-top: 30px;
      font-size: 0.9em;
      color: #aaa;
    }
  </style>
</head>
<body>
  <h1>🌿 Sensor Ambiental ()=====" +
                            String(customNombre) + R"=====()</h1>
  <p><i>La página se actualiza cada 10 segundos para mostrar el estado más reciente.</i></p>
  <p> </p>
  <h3>Ubicación 📌</h3>
  <p><b>Latitud:</b> )=====" +
                            String(customLatStr) + R"=====(</p>
  <p><b>Longitud:</b> )=====" +
                            String(customLonStr) + R"=====(</p>
  <p><b>Altura:</b> )=====" +
                            String(customAltStr) + R"=====(</p>

  <h3>Conexiones 📡</h3>
  <p><b>WiFi:</b> )=====" + WiFi.SSID() +
                            R"=====(</p>
  <p><b>IP:</b> )=====" + WiFi.localIP().toString() +
                            R"=====(</p>
  <p><b>MAC:</b> )=====" + WiFi.macAddress() +
                            R"=====(</p>
  <p><b>Estado:</b> )=====" +
                            (WiFi.status() == WL_CONNECTED
                                 ? "Conectado ✅"
                                 : "Desconectado ❌") +
                            R"=====(</p>

  <h3>Señal WiFi 📶</h3>
  <p><b>RSSI:</b> )=====" + String(rssi) +
                            R"=====( dBm</p>
  <p><b>Calidad:</b> <span style='color: )=====" +
                            color + R"=====(;'>)=====" + calidad +
                            R"=====(</span></p>

  <p><a href='/reset'>[Desconectar WiFi]</a> ⚠️</p>
  <p><a href='/update'>[Actualización]</a> 📥</p>

  <footer>
    Firmware v)=====" + String(APP_VERSION) +
                            R"=====( &mdash; Sensor-Ambiente.local
  </footer>
</body>
</html>
)=====";

              server.send(200, "text/html; charset=utf-8", html);
            });

  server.on("/reset", HTTP_GET, []() {
    String html = R"====(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>⚠️ Reconfigurar Sensor</title>
      <style>
        body {
          font-family: Arial, Helvetica, sans-serif;
          background-color: #121212;
          color: #f0f0f0;
          padding: 20px;
        }
        h1 {
          color: #ff6b6b;
          border-bottom: 1px solid #444;
          padding-bottom: 10px;
        }
        p, li {
          line-height: 1.6;
        }
        ul {
          margin: 10px 0 20px 20px;
        }
        .button {
          background-color: #e74c3c;
          color: white;
          padding: 12px 20px;
          text-align: center;
          text-decoration: none;
          font-size: 16px;
          border: none;
          border-radius: 5px;
          cursor: pointer;
        }
        .button:hover {
          background-color: #c0392b;
        }
        a {
          color: #00aced;
        }
      </style>
    </head>
    <body>
      <h1>🔁 Reconfigurar Sensor</h1>
      <p>Este proceso eliminará <b>toda la configuración actual</b>, incluyendo:</p>
      <ul>
        <li>Red WiFi (SSID y contraseña)</li>
        <li>Nombre y ubicación del sensor</li>
      </ul>
      <p>Una vez reiniciado, el dispositivo activará su propia red WiFi llamada:</p>
      <p><b>Sensor Ambiente</b> 📡</p>
      <p>Conéctate desde tu celular o PC a esa red y accede al portal en:</p>
      <p><a href='http://192.168.4.1'>http://192.168.4.1</a></p>
      <p>Desde allí podrás configurar nuevamente el WiFi y los datos de ubicación.</p>
      <form action="/do_reset" method="GET">
        <button class="button" type="submit">Reiniciar Configuración</button>
      </form>
    </body>
    </html>
  )====";

    server.send(200, "text/html; charset=utf-8", html);
  });

  // --- ¡NUEVA PÁGINA DE TEST DE SENSORES! ---
  server.on("/test", HTTP_GET, []() {
    String ahtStatus, ahtTemp, ahtHum;
    String bmpStatus, bmpTemp, bmpPres;

    // --- LEER SENSOR AHT ---
    if (aht_ok) {
      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);  // Llenar los objetos de evento
      ahtStatus = "<span class='ok'>Detectado ✅</span>";
      ahtTemp = String(temp.temperature, 2);  // Temperatura con 2 decimales
      ahtHum =
          String(humidity.relative_humidity, 2);  // Humedad con 2 decimales
    } else {
      ahtStatus = "<span class='fail'>No Encontrado ❌</span>";
      ahtTemp = "N/A";
      ahtHum = "N/A";
    }

    // --- LEER SENSOR BMP280 ---
    if (bmp_ok) {
      bmpStatus = "<span class='ok'>Detectado ✅</span>";
      bmpTemp = String(bmp.readTemperature(), 2);
      bmpPres =
          String(bmp.readPressure() / 100.0F, 2);  // Dividir para obtener hPa
    } else {
      bmpStatus = "<span class='fail'>No Encontrado ❌</span>";
      bmpTemp = "N/A";
      bmpPres = "N/A";
    }

    // --- CONSTRUIR EL HTML ---
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <meta http-equiv='refresh' content='5'>
  <title>Test de Sensores</title>
  <style>
    body { background-color: #121212; color: #ffffff; font-family: Arial, sans-serif; margin: 20px; }
    h1, h2 { border-bottom: 1px solid #444; padding-bottom: 5px; }
    p { margin: 8px 0; font-size: 1.1em; }
    a { color: #1e90ff; text-decoration: none; }
    a:hover { text-decoration: underline; }
    .sensor-box { border: 1px solid #333; border-radius: 8px; padding: 15px; margin-bottom: 15px; background-color: #1e1e1e; }
    .ok { color: #2ecc71; font-weight: bold; }
    .fail { color: #e74c3c; font-weight: bold; }
  </style>
</head>
<body>
  <h1>🔬 Test de Sensores</h1>
  <p><i>Esta página se actualiza automáticamente cada 5 segundos.</i></p>

  <div class='sensor-box'>
    <h2>🌡️ Sensor AHT (Temp/Humedad)</h2>
    <p><b>Estado:</b> )=====" +
                  ahtStatus + R"=====(</p>
    <p><b>Temperatura:</b> )=====" +
                  ahtTemp + R"=====( &deg;C</p>
    <p><b>Humedad:</b> )=====" +
                  ahtHum + R"=====( %</p>
  </div>

  <div class='sensor-box'>
    <h2>💨 Sensor BMP280 (Temp/Presión)</h2>
    <p><b>Estado:</b> )=====" +
                  bmpStatus + R"=====(</p>
    <p><b>Temperatura:</b> )=====" +
                  bmpTemp + R"=====( &deg;C</p>
    <p><b>Presión:</b> )=====" +
                  bmpPres + R"=====( hPa</p>
  </div>
  
  <p><a href='/'>[Volver a la página principal]</a></p>

</body>
</html>
)=====";
    server.send(200, "text/html; charset=utf-8", html);
  });

  server.on("/do_reset", HTTP_GET, []() {
    server.send(200, "text/html",
                "<p>Reiniciando... Por favor espera</p>"
                "<p>Portal de Configuracion en 192.168.4.1</p>");
    delay(3000);
    wm.resetSettings();  // 🔁 Borra SSID, clave y entra en modo portal
    ESP.restart();
  });
}

void inicializarMDNS() {
  if (MDNS.begin("sensor-ambiente")) {  // Nombre del dispositivo
    Serial.println("✅ mDNS iniciado. Accede a: sensor-ambiente.local");
  } else {
    Serial.println("❌ Error al iniciar mDNS");
  }
}