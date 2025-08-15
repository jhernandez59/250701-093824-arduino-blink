// ------------------------------
// Archivo: sensores.cpp
// ------------------------------
#include "sensores.h"

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
bool aht_ok = false;
bool bmp_ok = false;
DatosSensores datosAnterior;

void inicializarSensores() {
  aht_ok = aht.begin();
  bmp_ok = bmp.begin(0x77);
  if (bmp_ok) {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
  }
}

DatosSensores leerSensores() {
  DatosSensores datos = {0, 0, 0, false, false};

  if (aht_ok) {
    sensors_event_t humedadEvent, tempEvent;
    if (aht.getEvent(&humedadEvent, &tempEvent)) {
      datos.temperatura = tempEvent.temperature;
      datos.humedad = humedadEvent.relative_humidity;
      datos.ahtValido = true;
    }
  }

  if (bmp_ok) {
    float presion = bmp.readPressure();
    if (!isnan(presion)) {
      datos.presion = presion / 100.0;
      datos.bmpValido = true;
      if (!datos.ahtValido)
        datos.temperatura = bmp.readTemperature();
    }
  }
  return datos;
}

bool datosHanCambiado(const DatosSensores& nuevosDatos) {
  return abs(nuevosDatos.temperatura - datosAnterior.temperatura) >= 0.2 ||
         abs(nuevosDatos.humedad - datosAnterior.humedad) >= 0.5 ||
         abs(nuevosDatos.presion - datosAnterior.presion) >= 1.0;
}
