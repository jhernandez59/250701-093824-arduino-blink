import { generarTendencia } from "../web/js/ui-interior.js"; // Importamos la función de tendencia

const apiKey = "c313eb999e2d697113ca276d4a4c4ffa";
let ultimoFetch = 0;
let datosCache = null;

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
export async function obtenerClimaExterior(lat, lon) {
  const ahora = Date.now();
  const TIEMPO_CACHE = 15 * 60 * 1000; // 15 minutos

  if (ahora - ultimoFetch < TIEMPO_CACHE && datosCache) {
    console.log("✅ Usando datos de clima exterior cacheados.");
    return datosCache;
  }

  console.log(
    "🌤️ Obteniendo nuevos datos del ambiente exterior (Clima + AQI)..."
  );

  // === 1. Preparamos las URLs para las dos llamadas ===
  const urlClima = `https://api.openweathermap.org/data/2.5/weather?lat=${lat}&lon=${lon}&units=metric&lang=es&appid=${apiKey}`;
  const urlAire = `https://api.openweathermap.org/data/2.5/air_pollution?lat=${lat}&lon=${lon}&appid=${apiKey}`;

  try {
    // === 2. Ejecutamos ambas peticiones EN PARALELO ===
    // Promise.all toma un array de promesas y espera a que todas se resuelvan.
    const [climaRes, aireRes] = await Promise.all([
      fetch(urlClima),
      fetch(urlAire),
    ]);

    // Verificamos si alguna de las respuestas falló (ej. error 404, 500)
    if (!climaRes.ok || !aireRes.ok) {
      throw new Error(
        `Error en las respuestas de la API: Clima ${climaRes.status}, Aire ${aireRes.status}`
      );
    }

    // === 3. Extraemos los datos JSON de ambas respuestas, también en paralelo ===
    const [climaData, aireData] = await Promise.all([
      climaRes.json(),
      aireRes.json(),
    ]);

    // === 4. Procesamos y estructuramos los datos recibidos ===

    // --- Procesar datos del CLIMA ---
    const temp = climaData.main?.temp ?? 0;
    const humedad = climaData.main?.humidity ?? 0;
    const clima = {
      ciudad: climaData.name ?? "-",
      condicion: capitalizar(climaData.weather?.[0]?.description ?? "-"),
      temperatura: temp,
      sensacion: climaData.main?.feels_like ?? 0,
      humedad: humedad,
      presion: climaData.main?.pressure ?? 0,
      rocio: calcularPuntoRocio(temp, humedad),
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
    datosCache = {
      clima: clima,
      calidad: calidad,
      timestamp: ahora,
    };

    ultimoFetch = ahora;
    console.log(
      "✅ Datos de ambiente exterior obtenidos y cacheados:",
      datosCache
    );
    return datosCache;
  } catch (error) {
    console.error("❌ Error al obtener datos del ambiente exterior:", error);
    // Si algo falla (la red, una de las API, el JSON), devolvemos null
    // para que la UI sepa que no hay datos.
    return null;
  }
}
