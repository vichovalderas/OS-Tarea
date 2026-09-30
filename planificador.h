#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <unistd.h>     
#include <sys/wait.h>   
#include <csignal>      
#include <cstring>
#include "reader.h"

using namespace std;

enum Estado{
    PENDIENTE,     //si tiene dependencias por terminar
    LISTO,         //si no tiene dependencias por terminar
    EN_EJECUCION,  //se hace el fork para realizar la tarea
    COMPLETADO,    //se acaba la tarea
    FALLIDO,       //falla
    CANCELADO      //cancelar si alguna dependencia falla
};
struct ActividadEjecucion{
    Actividad data;
    Estado estado = PENDIENTE;
    pid_t pid = -1;
    int pipe_read_fd = -1;
};

//referencia para la seremi
vector<ActividadEjecucion>* g_grafo_ptr = nullptr;
void interrupt(int sig){
    (void)sig;//para que no salte alerta en el compilador
    cout << "LLEGO LA SEREMI" << endl;
    if (g_grafo_ptr != nullptr){
        for (auto& act : *g_grafo_ptr){
            if (act.estado == EN_EJECUCION && act.pid > 0){
                cout << "deteniendo :" << act.data.name << " / PID: " << act.pid <<endl;
                kill(act.pid, SIGKILL);
                act.estado = CANCELADO;
            }
        }
    }
    exit(130);
}
void abortRama(vector<ActividadEjecucion>& grafo, const string& id_fallido){
    vector<string> fallidos;
    fallidos.push_back(id_fallido);
    bool aux = true;

    while (aux){
        aux = false;
        for (auto& act : grafo){
            if (act.estado == PENDIENTE || act.estado == LISTO){
                for (const string& dep : act.data.dependencies){
                    if (find(fallidos.begin(), fallidos.end(), dep) != fallidos.end()){
                        act.estado = CANCELADO;
                        fallidos.push_back(act.data.id); 
                        cout << "Fallo en: "<< act.data.id << endl;
                        aux = true;
                        break;
                    }
                }
            }
        }
    }
}

bool ejecutarActividad(ActividadEjecucion& act){
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1){
        cerr << "Error al crear pipe para " << act.data.id << endl;
        return false;
    }

    pid_t pid = fork();

    if (pid < 0){
        cerr << "Error en fork para " << act.data.id << endl;
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return false;
    }

    if (pid == 0){ 
        //proceso hijo
        close(pipe_fd[0]);
        usleep(act.data.time_ms * 1000); //de mili a microsegundo
        string msg = "OK:" + act.data.id;
        write(pipe_fd[1], msg.c_str(), msg.length());
        close(pipe_fd[1]);
        exit(0); 
    }

    //proceso padre
    close(pipe_fd[1]);
    act.pid = pid;
    act.pipe_read_fd = pipe_fd[0];
    act.estado = EN_EJECUCION;
    cout << "Ejecutando actividad id: " << act.data.id << " | PID: " << pid << " | Duracion: " << act.data.time_ms << "ms" << endl;

    return true;
}

int countTareas(const vector<ActividadEjecucion>& grafo) {
    int cont = 0;
    for (const auto& act : grafo) {
        if (act.estado == COMPLETADO || act.estado == FALLIDO || act.estado == CANCELADO) {
            cont++;
        }
    }
    return cont;
}

void ejecutarPlanificador(vector<Actividad>& plan_inicial, int K){
    //para interrupcion
    struct sigaction sa;
    sa.sa_handler = interrupt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    vector<ActividadEjecucion> grafo;
    for (const auto& a : plan_inicial){
        ActividadEjecucion act;
        act.data = a;
        act.estado = (a.dependencias_pendientes == 0) ? LISTO : PENDIENTE;
        grafo.push_back(act);
    }

    int procesos_activos = 0;
    int total_tareas = grafo.size();
    while (countTareas(grafo) < total_tareas) {
        for (auto& act : grafo){
            if (act.estado == LISTO && procesos_activos < K){
                if (ejecutarActividad(act)){
                    procesos_activos++;
                }
            }
        }
        if (procesos_activos > 0){
            int status;
            pid_t pid_terminado = waitpid(-1, &status, 0);

            if (pid_terminado > 0){
                procesos_activos--;
                for (auto& act : grafo){
                    if (act.pid == pid_terminado){
                        
                        char buffer[128] ={0};
                        if (act.pipe_read_fd != -1){
                            read(act.pipe_read_fd, buffer, sizeof(buffer) - 1);
                            close(act.pipe_read_fd);
                            act.pipe_read_fd = -1;
                        }
                        string mensaje(buffer);
                        if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && mensaje.rfind("OK", 0) == 0){
                            act.estado = COMPLETADO;
                            cout << "Actividad completada. ID: " << act.data.id << endl;
                            for (auto& sig_act : grafo){
                                if (sig_act.estado == PENDIENTE){
                                    for (const string& dep : sig_act.data.dependencies){
                                        if (dep == act.data.id){
                                            sig_act.data.dependencias_pendientes--;
                                            if (sig_act.data.dependencias_pendientes == 0){
                                                sig_act.estado = LISTO;
                                            }
                                        }
                                    }
                                }
                            }
                        } else{
                            // Si el hijo falló o no envió confirmación válida
                            act.estado = FALLIDO;
                            cout << "Fallo en ID: " << act.data.id << "Abortando rama" <<endl;
                            abortRama(grafo, act.data.id);
                        }
                        break;
                    }
                }
            }
        }
    }

    cout << "RESUMEN" << endl;
    for (const auto& act : grafo){
        string est_str;
        switch(act.estado){
            case COMPLETADO: est_str = "COMPLETADO"; break;
            case FALLIDO:    est_str = "FALLIDO"; break;
            case CANCELADO:  est_str = "CANCELADO"; break;
            default:         est_str = "INCOMPLETO"; break;
        }
        cout << "Actividad " << act.data.id << ": " << act.data.name << "->" << est_str << endl;
    }
}

#endif