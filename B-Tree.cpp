#include <iostream>
#include <vector>
#include <queue>
using namespace std;

struct Nodo {
    bool esHoja;
    vector<int> claves;
    vector<Nodo*> hijos;
    Nodo(bool _esHoja) :esHoja(_esHoja) {}
};

class BTree {
    Nodo* root;
    int maxHijos, minClaves, maxClaves;
public:
    BTree(int _maxHijos);
    bool Search(Nodo* nodo, int valor);
    void Split(Nodo* nodo, int n);
    void Insert(int valor);
    void InsertNonFull(Nodo* nodo, int valor);
    void Remove(int valor);
    void RemoveImpl(Nodo* nodo, int valor);
    void Merge(Nodo* nodo, int i);
    void borrowFromLeft(Nodo* nodo, int i);
    void borrowFromRight(Nodo* nodo, int i);
    int getPredecessor(Nodo* nodo);
    int getSuccessor(Nodo* nodo);
    void Print();
};

BTree::BTree(int _maxHijos) {
    root = nullptr;
    maxHijos = _maxHijos;
    maxClaves = maxHijos - 1;
    minClaves = (maxHijos / 2) - 1;
}

bool BTree::Search(Nodo* nodo, int valor) {
    if (!nodo) return false;
    int i = 0;
    for (; i < nodo->claves.size() && valor > nodo->claves[i]; i++);
    if (i < nodo->claves.size() && nodo->claves[i] == valor) return true;
    if (nodo->esHoja) return false;
    else {
        return Search(nodo->hijos[i], valor);
    }
}

void BTree::Split(Nodo* nodo, int n) {
    Nodo* nodo_izq = nodo->hijos[n];
    Nodo* nodo_der = new Nodo(nodo_izq->esHoja);
    int pivot = nodo_izq->claves.size() / 2;
    int mid = nodo_izq->claves[pivot];
    nodo->claves.insert(nodo->claves.begin() + n, mid);
    int claves_size = nodo_izq->claves.size();
    for (int i = pivot + 1; i < claves_size; i++) {
        nodo_der->claves.push_back(nodo_izq->claves[i]);
    }
    nodo_izq->claves.erase(nodo_izq->claves.begin() + pivot,nodo_izq->claves.end());
    if (!nodo_izq->esHoja) {
        int hijos_size = nodo_izq->hijos.size();
        for (int i = pivot + 1; i < hijos_size; i++) {
            nodo_der->hijos.push_back(nodo_izq->hijos[i]);
        }
        nodo_izq->hijos.erase(nodo_izq->hijos.begin() + (pivot + 1), nodo_izq->hijos.end());
    }
    nodo->hijos.insert(nodo->hijos.begin() + (n + 1), nodo_der);
}

void BTree::Insert(int valor) {
    if (!root) root = new Nodo(true);
    if (root->claves.size() == maxClaves) {
        Nodo* new_root = new Nodo(false);
        new_root->hijos.push_back(root);
        Split(new_root, 0);
        root = new_root;
    }
    InsertNonFull(root, valor);
}

void BTree::InsertNonFull(Nodo* nodo, int valor) {
    int i = 0;
    for (; i < nodo->claves.size() && valor > nodo->claves[i]; i++);
    if (nodo->esHoja) {
        nodo->claves.insert(nodo->claves.begin() + i, valor);
        return;
    }
    if (i < nodo->hijos.size() && nodo->hijos[i]->claves.size() == maxClaves) {
        Split(nodo, i);
        if (valor > nodo->claves[i]) i++;
    }
    InsertNonFull(nodo->hijos[i], valor);
}

void BTree::Merge(Nodo* nodo, int i) {
    Nodo* nodo_izq = nodo->hijos[i];
    Nodo* nodo_der = nodo->hijos[i + 1];
    nodo_izq->claves.push_back(nodo->claves[i]);
    for (int j = 0; j < nodo_der->claves.size(); j++) {
        nodo_izq->claves.push_back(nodo_der->claves[j]);
    }
    if (!nodo_der->esHoja) {
        for (int j = 0; j < nodo_der->hijos.size(); j++) {
            nodo_izq->hijos.push_back(nodo_der->hijos[j]);
        }
    }
    nodo->claves.erase(nodo->claves.begin() + i);
    nodo->hijos.erase(nodo->hijos.begin() + (i + 1) );
    delete nodo_der;
    if (nodo == root && root->claves.size() == 0) {
        Nodo* nueva_root = root->hijos[0];
        delete root;
        root = nueva_root;
    }
}

int BTree::getPredecessor(Nodo* nodo) {
    while (!nodo->esHoja) {
        nodo = nodo->hijos.back();
    }
    return nodo->claves.back();
}

int BTree::getSuccessor(Nodo* nodo) {
    while (!nodo->esHoja) {
        nodo = nodo->hijos.front();
    }
    return nodo->claves.front();
}
void BTree::borrowFromLeft(Nodo* nodo, int i) {
    Nodo* hijo_izq = nodo->hijos[i - 1];
    Nodo* hijo_der = nodo->hijos[i];
    int clave = nodo->claves[i - 1];
    nodo->claves[i - 1] = hijo_izq->claves.back();
    hijo_der->claves.insert(hijo_der->claves.begin(), clave);
    hijo_izq->claves.pop_back();
    if (!hijo_izq->esHoja) {
        Nodo* ultHijo_izq = hijo_izq->hijos.back();
        hijo_der->hijos.insert(hijo_der->hijos.begin(), ultHijo_izq);
        hijo_izq->hijos.pop_back();
    }
}

void BTree::borrowFromRight(Nodo* nodo, int i) {
    Nodo* hijo_izq = nodo->hijos[i];
    Nodo* hijo_der = nodo->hijos[i + 1];
    int clave = nodo->claves[i];
    nodo->claves[i] = hijo_der->claves.front();
    hijo_izq->claves.push_back(clave);
    hijo_der->claves.erase(hijo_der->claves.begin());
    if (!hijo_der->esHoja) {
        hijo_izq->hijos.push_back(hijo_der->hijos.front());
        hijo_der->hijos.erase(hijo_der->hijos.begin());
    }
}

void BTree::RemoveImpl(Nodo* nodo, int valor) {
    int i = 0;
    for (; i < nodo->claves.size() && valor > nodo->claves[i]; i++);
    if (i < nodo->claves.size() && valor == nodo->claves[i]) {
        if (nodo->esHoja) { 
            nodo->claves.erase(nodo->claves.begin() + i); 
            return; 
        }
        else {
            Nodo* hijo_der = nodo->hijos[i + 1];
            Nodo* hijo_izq = nodo->hijos[i];
            if (hijo_izq->claves.size() > minClaves) {
                int predecesor = getPredecessor(hijo_izq);
                nodo->claves[i] = predecesor;
                RemoveImpl(hijo_izq, predecesor);
            }
            else if (hijo_der->claves.size() > minClaves) {
                int sucesor = getSuccessor(hijo_der);
                nodo->claves[i] = sucesor;
                RemoveImpl(hijo_der, sucesor);
            }
            else{
                Merge(nodo, i);
                RemoveImpl(nodo->hijos[i], valor);//No utilizamos hijos_izq, puede haber cambiado tras el merge
            }
        }
    }
    else{
        if (nodo->esHoja) return;
        if (nodo->hijos[i]->claves.size() == minClaves) {
            if (i > 0 && nodo->hijos[i - 1]->claves.size() > minClaves) borrowFromLeft(nodo, i);
            else if (i < nodo->claves.size() && nodo->hijos[i + 1]->claves.size() > minClaves) borrowFromRight(nodo, i);
            else { 
                if (i == 0) { 
                    Merge(nodo, i);
                    RemoveImpl(nodo->hijos[i], valor);
                    return;
                }
                else { 
                    Merge(nodo, i - 1); 
                    RemoveImpl(nodo->hijos[i - 1], valor);
                    return;
                }
            }
        }
        RemoveImpl(nodo->hijos[i], valor);
    }
}
void BTree::Remove(int valor) {
    if (!root) return;
    RemoveImpl(root, valor);
    if (root->claves.size() == 0) {
        Nodo* temp = root;
        if (root->esHoja) root = nullptr;//[1]
        else root = root->hijos[0];//[2]
        delete temp; // se elimina el nodo viejo, puesto que la ruta ahora será igual a uno de los casos anteriores [1] y [2]
    }
}

void BTree::Print() {
    if (!root) return;
    queue <Nodo*> q;
    q.push(root);
    while (!q.empty()) {
        int size = q.size();
        for (int i = 0; i < size; i++) {
            Nodo* nodo = q.front();
            q.pop();

            cout << "[ ";
            for (int j = 0; j < nodo->claves.size(); j++) {
                cout << nodo->claves[j] << " ";
            }
            cout << "] ";
            if (!nodo->esHoja) {
                for (int j = 0; j < nodo->hijos.size(); j++) {
                    q.push(nodo->hijos[j]);
                }
            }
        }
        cout << endl;
    }
}
int main() {
    BTree tree(4);

    tree.Insert(10);
    tree.Insert(20);
    tree.Insert(5);
    tree.Insert(6);
    tree.Insert(12);
    tree.Insert(30);
    tree.Insert(7);
    tree.Insert(17);

    cout << "Arbol:\n";
    tree.Print();

    tree.Remove(6);
    tree.Print();
    tree.Remove(30);
    tree.Print();
    tree.Remove(7);
    tree.Print();
    tree.Remove(20);
    tree.Print();

    return 0;
}
