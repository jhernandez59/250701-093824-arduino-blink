// Importa los módulos necesarios de Firebase
const functions = require("firebase-functions");
const admin = require("firebase-admin");

// Inicializa la app de administración de Firebase
admin.initializeApp();

/**
 * Cloud Function programada para limpiar registros antiguos del historial de presión.
 * Se ejecuta todos los días a las 3:00 AM (zona horaria de América/Bogotá).
 * Puedes cambiar el timezone a tu zona horaria local.
 * Lista de timezones: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
 */
exports.pruneOldPressureData = functions.pubsub
  .schedule("every day 03:00")
  .timeZone("America/Bogota") // <-- ¡IMPORTANTE! Cambia esto a tu zona horaria
  .onRun(async (context) => {
    console.log("Iniciando la tarea de limpieza de datos de presión...");

    // 1. Definir el límite de tiempo. Borraremos todo lo que sea más antiguo que 7 días.
    const DIAS_A_CONSERVAR = 7;
    const cutoffDate = new Date();
    cutoffDate.setDate(cutoffDate.getDate() - DIAS_A_CONSERVAR);
    const cutoffTimestamp = cutoffDate.getTime(); // Timestamp en milisegundos

    console.log(
      `Borrando registros anteriores a: ${cutoffDate.toISOString()} (${cutoffTimestamp})`
    );

    // 2. Obtener una referencia a la raíz de todos los sensores
    const sensorsRef = admin.database().ref("/sensores_en_tiempo_real");

    // 3. Leer la lista de todos los sensores (todas las MACs)
    const sensorsSnapshot = await sensorsRef.once("value");

    if (!sensorsSnapshot.exists()) {
      console.log("No se encontraron sensores. Tarea finalizada.");
      return null;
    }

    // 4. Preparar una lista de promesas para todas las operaciones de borrado
    const promises = [];

    // 5. Iterar sobre cada sensor encontrado
    sensorsSnapshot.forEach((sensorSnapshot) => {
      const macAddress = sensorSnapshot.key;
      const historyRef = sensorSnapshot.ref.child("pressure_history");

      console.log(`Revisando sensor: ${macAddress}`);

      // Creamos una consulta para encontrar los registros antiguos
      // Buscamos todos los registros cuyo timestamp sea MENOR O IGUAL al de corte.
      const oldRecordsQuery = historyRef
        .orderByChild("timestamp")
        .endAt(cutoffTimestamp);

      // Creamos una promesa de borrado para este sensor
      const deletePromise = oldRecordsQuery.once("value").then((snapshot) => {
        if (!snapshot.exists()) {
          console.log(
            ` -> No hay registros antiguos para borrar en ${macAddress}.`
          );
          return null;
        }

        console.log(
          ` -> Encontrados ${snapshot.numChildren()} registros antiguos para borrar en ${macAddress}.`
        );
        // El borrado se hace con .remove() en la referencia de la consulta
        return snapshot.ref.remove();
      });

      promises.push(deletePromise);
    });

    // 6. Ejecutar todas las promesas de borrado en paralelo
    await Promise.all(promises);

    console.log("Tarea de limpieza completada exitosamente.");
    return null;
  });
