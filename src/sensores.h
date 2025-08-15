// ------------------------------
// Archivo: sensores.h
// ------------------------------
#ifndef SENSORES_H
#define SENSORES_H

struct DatosSensores {
  float temperatura;
  float humedad;
  float presion;
  bool ahtValido;
  bool bmpValido;
};

#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

extern Adafruit_AHTX0 aht;
extern Adafruit_BMP280 bmp;
extern bool aht_ok;
extern bool bmp_ok;
extern DatosSensores datosAnterior;

void inicializarSensores();
DatosSensores leerSensores();
bool datosHanCambiado(const DatosSensores& actual);
#endif
