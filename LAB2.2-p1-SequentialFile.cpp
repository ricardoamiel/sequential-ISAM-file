#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <bits/stdc++.h> 

using namespace std;

struct Registro {
    char codigo[5];
    char nombre[20];
    char carrera[15];
    int ciclo;
    
    // Sobrecarga del operador de igualdad
    bool operator==(const Registro& other) const{
        return strcmp(codigo, other.codigo) == 0;
    }
    
    // Sobrecarga del operador menor que
    bool operator<(const Registro& other) const{
        return strcmp(codigo, other.codigo) < 0;
    }
    
    // Sobrecarga del operador mayor que
    bool operator>(const Registro& other) const{
        return strcmp(codigo, other.codigo) > 0;
    }
    
    // Función para obtener la clave del registro
    string get_key() const{
        return string(codigo,0,5);
    }
    
    // Constructor
    Registro(){
        strcpy(this->codigo, "");
        strcpy(this->nombre, "");
        strcpy(this->carrera, "");
    }
    
    Registro(char* codigo, char* nombre, char* carrera, int ciclo){
        strcpy(this->codigo, codigo);
        strcpy(this->nombre, nombre);
        strcpy(this->carrera, carrera);
        this->ciclo = ciclo;
    }
};

struct Index
{
    char codigo[5];
    streampos pointer;
    
    // Constructor
    Index(){
        strcpy(this->codigo, "");
        this->pointer = 0;
    }
    
    Index(const char* codigo, streampos pointer){
        strcpy(this->codigo, codigo);
        this->pointer = pointer;
    }
};

vector<Registro> sortR(vector<Registro> r){
    int i, j;
    Registro key;
    int n = r.size();
    for (int i = 1; i < n; i++){
        key = r[i];
        j = i - 1;

        while (j >= 0 && strcmp(r[j].codigo, key.codigo) > 0){
            r[j + 1] = r[j];
            j = j - 1;
        }
        r[j + 1] = key;
    }
    return r;
}

class SequentialFile{
    private:
        string filename;
        string auxFile;
        
    public:
        // Constructor
        SequentialFile(string filename){
            this->filename = filename;
            this->auxFile = "aux_" + filename;
        }
        
        // Función para insertar un registro en el archivo
        // Función para insertar un registro en el archivo
        void insertAll(vector<Registro> registros){
            ofstream file(this->filename, ios::binary | ios::app);

            registros = sortR(registros);
            
            for (int i = 0; i < registros.size(); i++){
                const Registro& registro = registros[i];
                
                // Escribir el registro en el archivo de datos
                file.write(reinterpret_cast<const char*>(&registro), sizeof(Registro));
            }

            file.close();
        }
        
        void add(Registro registro){
            ifstream file(this->filename, ios::binary | ios::in);
            file.seekg(0, ios::end);
            int mainRecords = file.tellg() / sizeof(Registro);

            ofstream aux(this->auxFile, ios::binary | ios::app);
            aux.write(reinterpret_cast<const char*>(&registro), sizeof(Registro));

            aux.close();

            ifstream auxRead(this->auxFile, ios::binary | ios::in);

            auxRead.seekg(0, ios::end);
            int auxRecords = auxRead.tellg() / sizeof(Registro);

            if (auxRecords >= log2(mainRecords)){
                vector<Registro> registros;

                file.seekg(0, ios::beg);
                while (file.peek() != EOF){
                    Registro reg;
                    file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
                    registros.push_back(reg);
                }

                file.close();

                auxRead.seekg(0, ios::beg);
                
                while (auxRead.peek() != EOF){
                    Registro reg;
                    auxRead.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
                    registros.push_back(reg);
                }

                registros = sortR(registros);

                ofstream fileWrite(this->filename, ios::binary | ios::out);
                fileWrite.seekp(0, ios::beg);

                for (int i = 0; i < registros.size(); i++){
                    const Registro& reg = registros[i];
                    fileWrite.write(reinterpret_cast<const char*>(&reg), sizeof(Registro));
                }

                fileWrite.close();
                auxRead.close();
                remove(this->auxFile.c_str());
            }
        }
        
        // Función para buscar un registro con una clave dada
        Registro search(string key){
            ifstream file(this->filename, ios::binary | ios::in);

            // Determinar el número de registros en el archivo
            file.seekg(0, ios::end);
            int numRecords = file.tellg() / sizeof(Registro);

            Registro registro;

            int low = 0, high = numRecords - 1;
            while (low <= high) {
                int mid = low + (high - low) / 2;
                file.seekg(mid * sizeof(Registro));
                file.read(reinterpret_cast<char*>(&registro), sizeof(Registro));
                
                int cmp = strcmp(key.c_str(), registro.codigo);
                if (cmp == 0)
                {
                    file.close();
                    return registro;
                }
                else if (cmp < 0)
                {
                    high = mid - 1;
                }
                else
                {
                    low = mid + 1;
                }
            }
            file.close();

            ifstream aux(this->auxFile, ios::binary | ios::in);
            aux.seekg(0, ios::beg);

            while (aux.tellg() != EOF){
                aux.read(reinterpret_cast<char*>(&registro), sizeof(Registro));
                if (strcmp(key.c_str(), registro.codigo) == 0){
                    aux.close();
                    return registro;
                }
            }

            return Registro();
        }
        
        // Función para buscar registros dentro de un rango dado de claves
        vector<Registro> rangeSearch(string begin, string end){
            vector<Registro> registros;
            ifstream file(this->filename, ios::binary | ios::in);
            int numRecords = file.tellg() / sizeof(Registro);
            Registro registro;

            int low = 0, high = numRecords - 1;
            int mid = 0;
            while (low <= high) {
                mid = low + (high - low) / 2;
                file.seekg(mid * sizeof(Registro));
                file.read(reinterpret_cast<char*>(&registro), sizeof(Registro));
                
                int cmp = strcmp(begin.c_str(), registro.codigo);
                if (cmp == 0)
                {
                    break;
                }
                else if (cmp < 0)
                {
                    high = mid - 1;
                }
                else
                {
                    low = mid + 1;
                }
            }
            
            file.seekg(mid, ios::beg);
            while (file.tellg() != EOF){
                file.read(reinterpret_cast<char*>(&registro), sizeof(Registro));
                if (strcmp(registro.codigo, end.c_str()) > 0){
                    break;
                }

                if (strcmp(registro.codigo, end.c_str()) <= 0 && strcmp(registro.codigo, begin.c_str()) >= 0){
                    registros.push_back(registro);
                }
            }
            file.close();

            ifstream aux(this->auxFile, ios::binary | ios::in);

            aux.seekg(0, ios::beg);
            while (aux.tellg() != EOF){
                aux.read(reinterpret_cast<char*>(&registro), sizeof(Registro));
                if (strcmp(registro.codigo, end.c_str()) <= 0 && strcmp(registro.codigo, begin.c_str()) >= 0){
                    registros.push_back(registro);
                }
            }

            registros = sortR(registros);

            return registros;
        }
};

int main(){
    SequentialFile file("data.dat");

    vector<Registro> registros;
    registros.push_back(Registro("001", "Juan Perez", "Ingenieria", 5));
    registros.push_back(Registro("003", "Carlos Ramirez", "Derecho", 4));
    registros.push_back(Registro("002", "Maria Lopez", "Medicina", 6));
    registros.push_back(Registro("005", "Pedro Sanchez", "Economia", 5));
    registros.push_back(Registro("004", "Ana Martinez", "Arquitectura", 7));

    file.insertAll(registros); // FUNCIONA

    file.add(Registro("008", "Lucia Torres", "Derecho", 6)); // FUNCIONA
    file.add(Registro("006", "Jose Rodriguez", "Ingenieria", 7)); // FUNCIONA
    file.add(Registro("007", "Luisa Garcia", "Medicina", 8)); // FUNCIONA
    file.add(Registro("009", "Pedro Gonzalez", "CS", 9)); // FUNCIONA

    Registro result = file.search("007"); // FUNCIONA
    if (strcmp(result.codigo, "") != 0){
        cout << "Registro encontrado: " << result.nombre << endl;
    } else {
        cout << "Registro no encontrado" << endl;
    }
    
    vector<Registro> rangeResult = file.rangeSearch("002", "004"); // A medias
    cout << "Registros encontrados en el rango:" << endl;
    for (const Registro& registro : rangeResult){
        cout << registro.nombre << endl;
    }
    
    return 0;
}