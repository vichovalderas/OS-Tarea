#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>    
#include <sys/wait.h>
#include <cstring>
#include "reader.h"

using namespace std;

enum Estado {
    PENDIENTE,     //si tiene dependencias por terminar
    LISTO,         //si no tiene dependencias por terminar
    EN_EJECUCION,  //se hace el fork para realizar la tarea
    COMPLETADO,    //se acaba la tarea
    FALLIDO        //falla
};

struct ActividadEjecucion {
    Actividad data;
    Estado estado = PENDIENTE;
    pid_t pid = -1;
};

bool ejecutarActividad(ActividadEjecucion& act, int canal_notificacion[2]) {
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "Error en" << act.data.id << endl;
        return false;
    }

    if (pid == 0) {
        //proceso hijo
        close(canal_notificacion[0]);

        usleep(act.data.time_ms * 1000); //de mili a microsegundo

        string mensaje = act.data.id + ":OK\n";
        
        write(canal_notificacion[1], mensaje.c_str(), mensaje.length());

        close(canal_notificacion[1]);
        exit(0);
    }

    //proceso padre
    act.pid = pid;
    act.estado = EN_EJECUCION;
    cout << "Ejecutando actividad id: " << act.data.id << " | PID: " << pid << " | Duracion: " << act.data.time_ms << "ms" << endl;

    return true;
}

void ejecutarPlanificador(vector<Actividad>& plan_inicial, int K) {
    vector<ActividadEjecucion> grafo;
    for (const auto& a : plan_inicial) {
        ActividadEjecucion act;
        act.data = a;
        if (a.dependencias_pendientes == 0) {
            act.estado = LISTO;
        } else {
            act.estado = PENDIENTE;
        }
        grafo.push_back(act);
    }

    int pipe_notificacion[2];
    if (pipe(pipe_notificacion) == -1) {
        cerr << "error en el pipe" << endl;
        return;
    }

    int procesos_activos = 0;
    int tareas_finalizadas = 0;
    int total_tareas = grafo.size();

    while (tareas_finalizadas < total_tareas) {
        for (auto& act : grafo) {
            if (act.estado == LISTO && procesos_activos < K) {
            if (ejecutarActividad(act, pipe_notificacion)) {
                procesos_activos++;
            }
            }
        }
        if (procesos_activos > 0) {
            int status;
            pid_t pid_finalizado = waitpid(-1, &status, 0);

            if (pid_finalizado > 0) {
                procesos_activos--;
                tareas_finalizadas++;
                string id_completado = "";
                for (auto& act : grafo) {
                    if (act.pid == pid_finalizado) {
                        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                            act.estado = COMPLETADO;
                            id_completado = act.data.id;
                            cout << "Actividad completada. ID: " << id_completado << endl;
                        } else {
                            act.estado = FALLIDO;
                            cout << "Fallo en ID: " << act.data.id << endl;
                        }
                        break;
                    }
                }
                if (!id_completado.empty()) {
                    for (auto& act : grafo) {
                        if (act.estado == PENDIENTE) {
                            // Revisamos si dependía de la tarea que acaba de terminar
                            for (const string& dep : act.data.dependencies) {
                                if (dep == id_completado) {
                                act.data.dependencias_pendientes--;
                                if (act.data.dependencias_pendientes == 0) {
                                    act.estado = LISTO;
                                }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    close(pipe_notificacion[0]);
    close(pipe_notificacion[1]);
    cout << "\n Plan finalizado" << endl;
}

#endif
