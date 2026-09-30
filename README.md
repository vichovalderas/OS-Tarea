# Tarea 1: Planificador Dieciochero

## Integrantes
Vicente Valderas

## Compilación y Ejecucion

### Compilación
El programa se puede compilar de las siguientes maneras:

```bash
g++ -Wall -Wextra -std=c++17 -lpthread main.cpp -o planificador
```

o llamando al Makefile:

```bash
make
```

### Ejecucion
El programa se ejecuta de la siguiente manera

```bash
./planificador plan.txt K
```
Donde K es la concurrencia maxima.

## Funciones Implementadas

1. reader.h

- readPlan(const string& plan_file): Lee el archivo plan.txt y formatea cada tarea dentro de una estructura de datos. Si una tarea no especifica su tiempo, le asigna un valor aleatorio entre 100 y 5000 ms.

2. planificador.h

- ejecutarPlanificador(vector<Actividad>& plan_inicial, int K): Orquestador principal. Mantiene el estado del DAG y controla el limite de procesos concurrentes (K). Ademas gestiona las notificaciones por pipes y actualiza las dependencias.

- ejecutarActividad(ActividadEjecucion& act): Crea una tuberia con pipe y ejecuta un nuevo proceso con fork(). El hijo simula la duracion de la tarea con usleep() y notifica su finalizacion escribiendo por el pipe.

- abortRama(vector<ActividadEjecucion>& grafo, const string& id_fallido): Implementa el aislamiento de errores. Propaga en cascada el estado CANCELADO únicamente a los nodos del DAG que dependían directa o indirectamente de la tarea fallida.

- interrupt(int sig): Manejador para la señal SIGINT (Ctrl+C). Intercepta la llegada de la autoridad (Seremi), recorre los procesos en estado EN_EJECUCION y les envía la señal SIGKILL para finalizar la ejecución de forma limpia.

- countTareas(const vector<ActividadEjecucion>& grafo): Función auxiliar que contabiliza cuántas actividades han finalizado en estados terminales (COMPLETADO, FALLIDO o CANCELADO).