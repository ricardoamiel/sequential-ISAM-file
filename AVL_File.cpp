#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

template<typename TK>
struct Record {
    TK id;
    string name;
};

template<typename TK>
struct AVLNode {
    Record<TK> record;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const Record<TK>& data){
        record = data;
        left = nullptr;
        right = nullptr;
        height = 1;
    }
};

template<typename TK>
class AVLFile {
private:
    string filename;
    long pos_root;
    AVLNode<TK>* root;

public:
    AVLFile(string filename) : filename(filename), pos_root(0), root(nullptr) {
        ifstream file(filename, ios::binary);
        if (file.is_open()) {
            file.read((char*)&pos_root, sizeof(pos_root));
            file.close();
        }
    }

    ~AVLFile() {
        ofstream file(filename, ios::binary);
        if (file.is_open()) {
            file.write((char*)&pos_root, sizeof(pos_root));
            file.close();
        }
    }

    Record<TK> find(TK key) {
        AVLNode<TK>* node = findNode(root, key);
        if (node) {
            return node->record;
        } else {
            return {};
        }
    }

    void insert(const Record<TK>& record) {
        root = insertNode(root, record);
        writeNode(root, pos_root);
    }

    void remove(TK key) {
        root = deleteNode(root, key);
        writeNode(root, pos_root);
    }

    vector<Record<TK>> inorder() {
        vector<Record<TK>> records;
        inorderTraversal(root, records);
        return records;
    }

private:
    AVLNode<TK>* findNode(AVLNode<TK>* node, TK key) {
        if (node == nullptr)
            return nullptr;

        if (key < node->record.id)
            return findNode(node->left, key);
        else if (key > node->record.id)
            return findNode(node->right, key);
        else
            return node;
    }

    int height(AVLNode<TK>* node) {
        if (node == nullptr)
            return 0;
        return node->height;
    }

    int balanceFactor(AVLNode<TK>* node) {
        if (node == nullptr)
            return 0;
        return height(node->left) - height(node->right);
    }

    void updateHeight(AVLNode<TK>* node) {
        if (node == nullptr)
            return;
        node->height = 1 + max(height(node->left), height(node->right));
    }

    AVLNode<TK>* rotateRight(AVLNode<TK>* y) {
        AVLNode<TK>* x = y->left;
        AVLNode<TK>* z = x->right;

        x->right = y;
        y->left = z;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    AVLNode<TK>* rotateLeft(AVLNode<TK>* x) {
        AVLNode<TK>* y = x->right;
        AVLNode<TK>* z = y->left;

        y->left = x;
        x->right = z;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    AVLNode<TK>* insertNode(AVLNode<TK>* node, const Record<TK>& record) {
        if (node == nullptr)
            return new AVLNode<TK>(record);

        if (record.id < node->record.id)
            node->left = insertNode(node->left, record);
        else if (record.id > node->record.id)
            node->right = insertNode(node->right, record);
        else
            return node;

        updateHeight(node);

        int balance = balanceFactor(node);

        if (balance > 1 and record.id < node->left->record.id)
            return rotateRight(node);

        if (balance < -1 and record.id > node->right->record.id)
            return rotateLeft(node);

        if (balance > 1 and record.id > node->left->record.id) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        if (balance < -1 and record.id < node->right->record.id) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    AVLNode<TK>* deleteNode(AVLNode<TK>* node, TK key) {
        if (node == nullptr)
            return node;

        if (key < node->record.id)
            node->left = deleteNode(node->left, key);
        else if (key > node->record.id)
            node->right = deleteNode(node->right, key);
        else {
            if (node->left == nullptr || node->right == nullptr) {
                AVLNode<TK>* temp = node->left ? node->left : node->right;

                if (temp == nullptr) {
                    temp = node;
                    node = nullptr;
                } else
                    *node = *temp;

                delete temp;
            } else {
                AVLNode<TK>* temp = minValueNode(node->right);
                node->record = temp->record;
                node->right = deleteNode(node->right, temp->record.id);
            }
        }

        if (node == nullptr)
            return node;

        updateHeight(node);

        int balance = balanceFactor(node);

        if (balance > 1 && balanceFactor(node->left) >= 0)
            return rotateRight(node);

        if (balance > 1 && balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        if (balance < -1 && balanceFactor(node->right) <= 0)
            return rotateLeft(node);

        if (balance < -1 && balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    AVLNode<TK>* minValueNode(AVLNode<TK>* node) {
        AVLNode<TK>* current = node;

        while (current->left != nullptr)
            current = current->left;

        return current;
    }

    void inorderTraversal(AVLNode<TK>* node, vector<Record<TK>>& records) {
        if (node == nullptr)
            return;

        inorderTraversal(node->left, records);
        records.push_back(node->record);
        inorderTraversal(node->right, records);
    }

    void writeNode(AVLNode<TK>* node, long& pos) {
        ofstream file(filename, ios::binary | ios::trunc);
        if (!file.is_open()) {
            cerr << "Error: No se puede abrir el archivo para escribirlo." << endl;
            exit(1);
        }

        if (node == nullptr) {
            file.seekp(pos);
            pos = file.tellp();
            file.close();
            return;
        }

        file.seekp(pos);
        file.write((char*)node, sizeof(AVLNode<TK>));
        pos = file.tellp();

        writeNode(node->left, pos);
        writeNode(node->right, pos);

        file.close();
    }
};

int main() {
    AVLFile<int> avlFile("data.bin");

    avlFile.insert({3, "Josue"});
    avlFile.insert({2, "Ricardo"});
    avlFile.insert({1, "Luis"});

    Record<int> result = avlFile.find(2);
    if (result.id != -1) {
        cout << "Registro encontrado: " << result.id << ", " << result.name << endl;
    } else {
        cout << "Registro no encontrado." << endl;
    }

    vector<Record<int>> orderedRecords = avlFile.inorder();
    cout << "Todos los registros ordenados:" << endl;
    for (const auto& record : orderedRecords) {
        cout << record.id << ": " << record.name << endl;
    }

    avlFile.remove(2);
    cout << "Después de eliminar el registro con ID 2:" << endl;
    orderedRecords = avlFile.inorder();
    for (const auto& record : orderedRecords) {
        cout << record.id << ": " << record.name << endl;
    }

    return 0;
}

