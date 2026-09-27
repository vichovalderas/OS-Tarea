#ifndef READER_H
#define READER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;

struct Actividad {
    string id;
    string name;
    int time_ms;
    vector<string> dependencies;  
    int dependencias_pendientes = 0; 
};


inline void trim(string &s) { //limpia espacios en blanco
    s.erase(0, s.find_first_not_of(" \t\r\n"));
    s.erase(s.find_last_not_of(" \t\r\n") + 1);
}

vector<Actividad> readPlan(const string& plan_file) {
    vector<Actividad> Plan;
    ifstream file(plan_file);
    
    if(!file.is_open()){
        cerr << "Error al intentar abrir " << plan_file << endl;
        return Plan;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(100, 5000);

    string row;
    while(getline(file, row)){
        if(row.empty()) continue;

        stringstream data(row);
        Actividad a;
        string temp_time;
        string dependencias;

        getline(data, a.id, ':');
        getline(data, a.name, ':');
        getline(data, temp_time, ':');
        getline(data, dependencias);

        //limpiar espacios
        trim(a.id);
        trim(a.name);
        trim(temp_time);
        trim(dependencias);

        //definir tiempo
        if (temp_time.empty()) {
            //generar si no se define en el txt
            a.time_ms = distrib(gen); 
        } else {
            a.time_ms = stoi(temp_time);
        }

        if (!dependencias.empty()) {
            stringstream ss_dep(dependencias);
            string temp_dep;

            while (getline(ss_dep, temp_dep, ',')) {
                trim(temp_dep); // Limpiar espacios de la dependencia
                if (!temp_dep.empty()) {
                    a.dependencies.push_back(temp_dep);
                }
            }
        }
        a.dependencias_pendientes = a.dependencies.size();
        
        Plan.push_back(a);
    }
    return Plan;
}

#endif