#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <random>

using namespace std;

struct Actividad {
    int id;
    string name;

    int time_ms;
    vector<int> dependencies;  
    int dependencias_pendientes = 0; 
};

int main(int argc, char* argv[]) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(100, 5000);

    if (argc != 3) {
        cerr << "Entregar argumentos" << argv[0] << " <archivo_plan.txt> <K" << endl;
        return 1;
    }

    string plan_file = argv[1];
    int K = stoi(argv[2]); 

    cout << "Plan:" << plan_file << endl;
    cout << "Limite ce concurrencia: " << K << endl;

    ifstream file(plan_file);
    if(!file.is_open()){
        cout<<"Error al intentar abrir "<<plan_file<<endl;
        return 1;
    }
    vector<Actividad> Plan;
    string row;
    while(getline(file, row)){
        if(row.empty()) continue;

        stringstream data(row);
        Actividad a;
        string temp_id;
        string temp_time;
        string dependencias;

        getline(data, temp_id, ':');
        getline(data, a.name, ':');
        getline(data, temp_time, ':');
        getline(data, dependencias);

        a.id = stoi(temp_id);

        //definir tiempo de la tarea
        if (temp_time.empty()) {
            //si no se define en el .txt
            a.time_ms = distrib(gen); 
        } else {
            //si se define su tiempo en el .txt
            a.time_ms = stoi(temp_time);
        }


        if (dependencias.empty()) {
            a.dependencias_pendientes = 0;
        } else {
            stringstream ss_dep(dependencias);
            string temp_dep;

            // Separamos por comas ',' cada número dentro de esta sección
            int pendientes = 0;
            while (getline(ss_dep, temp_dep, ',')) {
                if (!temp_dep.empty()) {
                    a.dependencies.push_back(stoi(temp_dep));
                    pendientes++;
                    a.dependencias_pendientes += pendientes;
                }
            }
        }
        Plan.push_back(a);
    }

    for (const auto& a : Plan) {
        std::cout << "NOMBRE: " << a.name << " | Dependencias (" << a.dependencies.size() << "): ";
        for (int n : a.dependencies) {
            std::cout << n << " ";
        }
        std::cout << "\n";
    }

    return 0;
}