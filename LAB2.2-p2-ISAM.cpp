#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>

// Definir B_d y B_i
const int B_d = 5;  // Assumed data page block factor
const int B_i = 3;  // Assumed index page block factor

using namespace std;

// Estructura para los registros en las páginas de datos.
struct Registro {
    string key;
    string data;

    string serialize() const {
        return key + "," + data;
    }

    static Registro deserialize(const string& str) {
        stringstream ss(str);
        string key, data;
        getline(ss, key, ',');
        getline(ss, data);
        return {key, data};
    }
};

// Clase base abstracta para las páginas.
struct Page {
    string filename; // Nombre de archivo para guardar y cargar la página

    Page(const string& fname) : filename(fname) {}

    virtual void writeToFile() const = 0;
    virtual void readFromFile() = 0;
    virtual ~Page() {}
};

// Página de datos que contiene registros, el archivo y un enlace a una página de desbordamiento.
struct DataPage : public Page {
    vector<Registro> registros;
    string overflowPageFilename;
    DataPage* nextOverflowPage;

    DataPage(const string& fname) : Page(fname), overflowPageFilename("") , nextOverflowPage(nullptr){}

    bool isFull() {
        return registros.size() >= B_d; // B_d es el factor de bloqueo de datos
    }

    void insert(const Registro& registro) {
        registros.push_back(registro);
    }

    Registro find(string key) {
        for (auto& reg : registros) {
            if (reg.key == key) return reg;
        }
        throw runtime_error("Registro not found");
    }

    vector<Registro> findRange(string beginKey, string endKey) {
        vector<Registro> result;
        for (auto& reg : registros) {
            if (reg.key >= beginKey && reg.key <= endKey) {
                result.push_back(reg);
            }
        }
        return result;  
    }

    DataPage* getNextOverflowPage() {
        if (overflowPageFilename.empty()) return nullptr;
        DataPage* overflowPage = new DataPage(overflowPageFilename);
        overflowPage->readFromFile();
        return overflowPage;
    }
    
    void writeToFile() const override {
        ofstream file(filename, ios::binary);
        for (const auto& reg : registros) {
            file.write(reg.serialize().c_str(), reg.serialize().size());
            file.write("\n", 1);
        }
        if (!overflowPageFilename.empty()) {
            file.write("overflow,", 9);
            file.write(overflowPageFilename.c_str(), overflowPageFilename.size());
            file.write("\n", 1);
        }
        file.close();
    }
    
    void readFromFile() override {
        ifstream file(filename, ios::binary);
        string line;
        registros.clear();
        while (getline(file, line)) {
            if (line.rfind("overflow,", 0) == 0) {
                overflowPageFilename = line.substr(9);
                continue;
            }
            registros.push_back(Registro::deserialize(line));
        }
        file.close();
    }
    
    ~DataPage() {
        delete nextOverflowPage;
    }
};

struct IndexPage : public Page {
    struct Entry {
        string key;
        Page* childPage;
    };

    vector<Entry> entries;
    
    IndexPage(string key) : Page(key) {}

    Page* findChildPage(string key) {
        // Find the appropriate child page for the given key
        for (size_t i = 0; i < entries.size(); i++) {
            if (i + 1 == entries.size() || entries[i + 1].key > key) {
                return entries[i].childPage;
            }
        }
        return nullptr;
    }
    
    void writeToFile() const override {
        ofstream file(filename, ios::binary);
        for (const auto& entry : entries) {
            file.write(entry.key.c_str(), entry.key.size());
            file.write(",", 1);
            file.write(entry.childPage->filename.c_str(), entry.childPage->filename.size());
            file.write("\n", 1);
        }
        file.close();
    }
    
    void readFromFile() override {
        ifstream file(filename, ios::binary);
        string line;
        entries.clear();
        while (getline(file, line)) {
            stringstream ss(line);
            string key, childPageFilename;
            getline(ss, key, ',');
            getline(ss, childPageFilename);
            DataPage* childPage = new DataPage(childPageFilename);
            childPage->readFromFile();
            entries.push_back({key, childPage});
        }
        file.close();
    }
    
    
    ~IndexPage() {
        for (auto& entry : entries) {
            delete entry.childPage;
        }
    }
};

// Funciones auxiliares de manejo de páginas.
Page* searchPage(string key, IndexPage* root) {
    Page* currentPage = root;
    while (dynamic_cast<IndexPage*>(currentPage)) {
        IndexPage* currentIndexPage = dynamic_cast<IndexPage*>(currentPage);
        currentPage = currentIndexPage->findChildPage(key);
    }
    return currentPage;
}

// Simulate read and write operations to "storage"
Page* readPage(Page* page) {
    return page; // In reality, this would involve I/O operations.
}

void writePage(Page* page) {
    // Simulate writing a page back to disk (no actual I/O here).
}

DataPage* dynamic_cast_DataPage(Page* page) {
    return dynamic_cast<DataPage*>(page);
}

void add(Registro registro, IndexPage* rootPage) {
    // Busca la página en la que debe ir el registro.
    DataPage* page = dynamic_cast_DataPage(searchPage(registro.key, rootPage));

    // Inserta el registro en la página o en una página de desbordamiento.
    while (page->isFull()) {
        if (page->overflowPageFilename.empty()) {
            // Crea un nuevo nombre de archivo para la página de desbordamiento.
            page->overflowPageFilename = page->filename + "_overflow";
        }
        if (page->getNextOverflowPage() == nullptr) {
            // Carga o crea la página de desbordamiento.
            page->nextOverflowPage = new DataPage(page->overflowPageFilename);
            page->nextOverflowPage->readFromFile();
        }
        page = page->getNextOverflowPage();
    }
    page->insert(registro);

    // Escribe la página de datos y todas las páginas de desbordamiento a archivos.
    while (page != nullptr) {
        page->writeToFile();
        page = dynamic_cast<DataPage*>(page->getNextOverflowPage());
    }
}

// Función para buscar registros.
vector<Registro> search(string key, IndexPage* rootPage) {
    vector<Registro> foundRecords;
    DataPage* page = dynamic_cast_DataPage(searchPage(key, rootPage));

    while (page != nullptr) {
        try {
            Registro found = page->find(key);
            foundRecords.push_back(found);
        } catch (const runtime_error& e) {
            // Continúa si el registro no se encuentra en la página actual.
        }
        page = page->getNextOverflowPage();
    }

    return foundRecords;
}

// Función para búsqueda por rango.
vector<Registro> rangeSearch(string beginKey, string endKey, IndexPage* rootPage) {
    vector<Registro> results;
    DataPage* page = dynamic_cast_DataPage(searchPage(beginKey, rootPage));

    while (page != nullptr && !page->registros.empty() && page->registros.front().key <= endKey) {
        vector<Registro> partialResults = page->findRange(beginKey, endKey);
        results.insert(results.end(), partialResults.begin(), partialResults.end());
        page = page->getNextOverflowPage();
    }

    return results;
}

int main() {
    // Simulación de la creación de índices y datos.
    IndexPage* rootPage = new IndexPage("root.dat");
    rootPage->entries.push_back({"A", new DataPage("dataA.dat")});
    rootPage->entries.push_back({"C", new DataPage("dataC.dat")});
    rootPage->entries.push_back({"E", new DataPage("dataE.dat")});
    rootPage->writeToFile();

    // Simulación de la inserción de registros.
    add({"A1", "Data A1"}, rootPage);
    add({"A2", "Data A2"}, rootPage);
    add({"A3", "Data A3"}, rootPage);
    add({"C1", "Data C1"}, rootPage);
    add({"C2", "Data C2"}, rootPage);
    add({"C3", "Data C3"}, rootPage);
    add({"E1", "Data E1"}, rootPage);
    add({"E2", "Data E2"}, rootPage);
    add({"E3", "Data E3"}, rootPage);

    // Simulación de la búsqueda de registros.
    vector<Registro> foundA = search("A2", rootPage);
    for (const auto& reg : foundA) {
        cout << "Found A2: " << reg.data << endl;
    }

    vector<Registro> foundRange = rangeSearch("C1", "E3", rootPage);
    for (const auto& reg : foundRange) {
        cout << "Found range: " << reg.data << endl;
    }

    delete rootPage;
    return 0;
}

/*
// Función para insertar un nuevo registro
void insert(int key, Registro registro) {
    // 1. Buscar la hoja que debería contener la clave
    // 2. Si la hoja tiene espacio, insertar la clave y el registro
    // 3. Si la hoja está llena, dividirla y propagar la clave de división hacia arriba en el índice
    // 4. Si un nodo interno se llena durante la propagación, dividirlo también
    // 5. Si la raíz se llena, crear una nueva raíz
}

// Función para buscar un registro
Registro search(int key) {
    // 1. Comenzar en la raíz del índice
    // 2. Buscar la clave en el nodo actual
    // 3. Si la clave se encuentra y el nodo es una hoja, devolver el registro
    // 4. Si la clave se encuentra y el nodo es un nodo interno, seguir el puntero correspondiente al hijo y repetir desde el paso 2
    // 5. Si la clave no se encuentra y el nodo es una hoja, la clave no está en el índice, devolver un error o un valor nulo
    // 6. Si la clave no se encuentra y el nodo es un nodo interno, seguir el puntero al hijo que debería contener la clave y repetir desde el paso 2
}

// Función para buscar un rango de registros
vector<Registro> rangeSearch(int beginKey, int endKey) {
    // 1. Comenzar en la raíz del índice
    // 2. Buscar la clave de inicio en el nodo actual
    // 3. Si la clave de inicio se encuentra y el nodo es una hoja, comenzar a recoger registros
    // 4. Si la clave de inicio se encuentra y el nodo es un nodo interno, seguir el puntero correspondiente al hijo y repetir desde el paso 2
    // 5. Si la clave de inicio no se encuentra y el nodo es una hoja, la clave de inicio no está en el índice, comenzar a recoger registros desde la primera clave mayor que la clave de inicio
    // 6. Si la clave de inicio no se encuentra y el nodo es un nodo interno, seguir el puntero al hijo que debería contener la clave de inicio y repetir desde el paso 2
    // 7. Continuar recogiendo registros hasta que se encuentre una clave mayor que la clave final
}
*/