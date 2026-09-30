#include <iostream>
#include "reader.h"
#include "planificador.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso ./planificador archivo_plan.txt K_concurrencia" << endl;
        return 1;
    }

    string plan_file = argv[1];
    int K = stoi(argv[2]); 

    if (K <= 0) {
        cerr << "Valor K no valido" << endl;
        return 1;
    }

    vector<Actividad> Plan = readPlan(plan_file);

    if (Plan.empty()) {
        cerr << "Error al cargar archivo" << endl;
        return 1;
    }
    cout << "Archivo: " << plan_file << endl;
    cout << "K: " << K << endl;
    cout << "Tareas totales: " << Plan.size() << endl;

    ejecutarPlanificador(Plan, K);

    return 0;
}