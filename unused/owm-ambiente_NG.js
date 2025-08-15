// js/owm-ambiente.js
// no funciona, los datos se deben almacenar de forma persistente
import { generarTendencia } from "../web/js/ui-interior.js"; // Importamos la función de tendencia

const apiKey = "c313eb999e2d697113ca276d4a4c4ffa";
let ultimoFetch = 0;
let datosCache = null;
let datosAnterioresCache = null;

// Calcular punto de rocío
function calcularPuntoRocio(tempC, humedad) {
  const a = 17.27,
    b = 237.7;
  const alpha = (a * tempC) / (b + tempC) + Math.log(humedad / 100);
  return (b * alpha) / (a - alpha);
}

function capitalizar(texto) {
  return texto.charAt(0).toUpperCase() + texto.slice(1);
}

function descripcionAQI(aqi) {
  switch (aqi) {
    case 1:
      return "Bueno";
    case 2:
      return "Aceptable";
    case 3:
      return "Moderado";
    case 4:
      return "Malo";
    case 5:
      return "Muy malo";
    default:
      return "Desconocido";
  }
}

// Hacemos que esta función esté disponible para ser llamada desde otros archivos
export async function obtenerAmbienteExterior(lat, lon) {
  const ahora = Date.now();
  const TIEMPO_CACHE = 15 * 60 * 1000; // 15 minutos

  // --- CAMBIO 1: LÓGICA DE CACHÉ MODIFICADA ---
  // Si el caché es válido, devolvemos inmediatamente la estructura correcta.
  if (ahora - ultimoFetch < TIEMPO_CACHE && datosCache) {
    console.log("✅ Usando datos cacheados.");
    // Devolvemos el caché actual como "datosActuales" y el caché anterior como "datosAnteriores".
    return {
      datosActuales: datosCache,
      datosAnteriores: datosAnterioresCache,
    };
  }

  // --- CAMBIO 2: LÓGICA DE GUARDADO DE DATOS ANTERIORES ---
  // Si llegamos aquí, significa que vamos a hacer un fetch.
  // Es el momento de mover el caché actual (si existe) a la variable de anteriores.
  if (datosCache) {
    datosAnterioresCache = datosCache;
    console.log("📊 Datos cacheados movidos a 'anteriores' para tendencia.");
  }

  console.log("🌤️ Obteniendo nuevos datos (OneCall + AQI + Geo)...");

  // === 1. Preparamos las NUEVAS URLs ===
  // Nota: usamos el endpoint 'onecall' y excluimos partes que no necesitamos para ahorrar datos.
  const urlOneCall = `https://api.openweathermap.org/data/3.0/onecall?lat=${lat}&lon=${lon}&exclude=minutely,daily,alerts&units=metric&lang=es&appid=${apiKey}`;
  const urlAire = `https://api.openweathermap.org/data/2.5/air_pollution?lat=${lat}&lon=${lon}&appid=${apiKey}`;
  const urlGeo = `https://api.openweathermap.org/geo/1.0/reverse?lat=${lat}&lon=${lon}&limit=1&appid=${apiKey}`;

  try {
    // === 2. Ejecutamos las peticiones EN PARALELO ===
    const [onecallRes, aireRes, geoRes] = await Promise.all([
      fetch(urlOneCall),
      fetch(urlAire),
      fetch(urlGeo),
    ]);

    // Verificamos si alguna de las respuestas falló (ej. error 404, 500)
    if (!onecallRes.ok || !aireRes.ok || !geoRes.ok) {
      throw new Error(
        `Error en API: OneCall ${onecallRes.status}, Aire ${aireRes.status}. Geo ${geoRes.status}`
      );
    }

    // === 3. Extraemos los datos JSON en paralelo ===
    const [onecallData, aireData, geoData] = await Promise.all([
      onecallRes.json(),
      aireRes.json(),
      geoRes.json(),
    ]);

    // === 4. Procesamos y estructuramos los datos recibidos ===

    // --- A. Procesar datos del CLIMA (ahora desde onecallData) ---
    const current = onecallData.current;
    const hourly = onecallData.hourly[0]; // Datos de la hora actual

    // La probabilidad de precipitación (pop) viene como un valor de 0 a 1.
    const probLluvia = hourly?.pop ?? 0;

    const clima = {
      ciudad: geoData[0]?.name ?? onecallData.timezone, // OneCall da el timezone, no el nombre de ciudad
      condicion: capitalizar(current.weather?.[0]?.description ?? "-"),
      temperatura: current.temp ?? 0,
      sensacion: current.feels_like ?? 0,
      humedad: current.humidity ?? 0,
      presion: current.pressure ?? 0,
      rocio: calcularPuntoRocio(current.temp, current.humidity),
      probLluvia: probLluvia, // ¡NUESTRO NUEVO DATO!
    };

    // --- Procesar datos del AIRE (AQI) ---
    // Hacemos un chequeo de seguridad por si la respuesta de aire no trae la lista
    const itemAire = aireData.list?.[0];
    const calidad = {
      aqi: itemAire?.main?.aqi ?? 0, // Usamos optional chaining por seguridad
      descripcion: itemAire
        ? descripcionAQI(itemAire.main.aqi)
        : "No disponible",
      componentes: itemAire?.components ?? {},
    };

    // === 5. Armamos la estructura final y la guardamos en caché ===
    // Creamos el nuevo objeto de datos actuales
    const nuevosDatosActuales = {
      clima: clima,
      calidad: calidad,
      timestamp: ahora,
    };

    // Actualizamos el timestamp del fetch y el caché principal
    ultimoFetch = ahora;
    datosCache = nuevosDatosActuales;

    console.log(
      "✅ Datos de ambiente exterior obtenidos y cacheados:",
      datosCache
    );

    // Devolvemos AMBOS: el actual y el anterior
    return {
      datosActuales: datosCache,
      datosAnteriores: datosAnterioresCache,
    };
  } catch (error) {
    console.error("❌ Error al obtener datos del ambiente exterior:", error);
    // Si algo falla (la red, una de las API, el JSON), devolvemos null
    // para que la UI sepa que no hay datos.
    return null;
  }
}
