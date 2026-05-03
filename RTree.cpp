#include <iostream>
#include <vector>
#include <limits>
using namespace std;

struct MBR {
    int xmin, xmax, ymax, ymin;
};

struct Punto {
    int x, y;
};

struct Nodo;

struct Entrada {
    MBR mbr;
    Nodo* hijo;
    Punto punto;
    Entrada() {
        hijo = nullptr;
        punto = { 0,0 };
    }
};

struct Nodo {
    Nodo* padre;
    vector<Entrada> entradas;
    bool esHoja;
    Nodo(bool _esHoja) :esHoja(_esHoja) {
        padre = nullptr;
    }
};

enum TipoSplit { QUADRATIC, LINEAR, EXHAUSTIVE, GREENE };

class RTree {
    int minEntrada, maxEntradas;
    TipoSplit tipoSplit;
    Nodo* root;
    MBR calcularMBR(Nodo* nodo);
    void ajustarPadre(Nodo* nodo);
    MBR unir(MBR rect1, MBR rect2);
    int area(MBR rect);
public:
    RTree(int max, TipoSplit tipo);
    Nodo* ChooseLeaf(Nodo* nodo, MBR rect);
    void AdjustTree(Nodo* nodo, Nodo* nuevo_nodo);
    pair<int, int> PickSeeds(Nodo* nodo);
    int PickNext(MBR rect1, MBR rect2, vector<Entrada>& pendientes);
    Nodo* QuadraticSplit(Nodo* nodo);
    Nodo* LinearSplit(Nodo* nodo);
    Nodo* ExhaustiveSplit(Nodo* nodo);
    Nodo* GreeneSplit(Nodo* nodo);
    Nodo* SplitNodo(Nodo* nodo);
    void Insert(Punto p, MBR rect);
    void Print();
};
RTree::RTree(int max, TipoSplit tipo) {
    root = nullptr;
    minEntrada = max/2;
    maxEntradas = max;
    tipoSplit = tipo;
}

MBR RTree::unir(MBR rect1, MBR rect2) {
    MBR rect;
    rect.xmin = min(rect1.xmin, rect2.xmin);
    rect.ymin = min(rect1.ymin, rect2.ymin);
    rect.xmax = max(rect1.xmax, rect2.xmax);
    rect.ymax = max(rect1.ymax, rect2.ymax);
    return rect;
}

int RTree::area(MBR rect) {
    return (rect.xmax - rect.xmin) * (rect.ymax - rect.ymin);
}

Nodo* RTree::ChooseLeaf(Nodo* nodo, MBR rect) {
    if (nodo->esHoja) return nodo;
    MBR rect_actual = nodo->entradas[0].mbr;
    int mejor_expansion = area(unir(rect_actual, rect)) - area(rect_actual);
    int indice{ 0 };
    for (int i = 1; i < nodo->entradas.size(); i++) {
        rect_actual = nodo->entradas[i].mbr;
        int expansion = area(unir(rect_actual, rect)) - area(rect_actual);
        if (mejor_expansion > expansion) {
            mejor_expansion = expansion;
            indice = i;
        }
        else if (mejor_expansion == expansion) {
            if (area(rect_actual) < area(nodo->entradas[indice].mbr)) {
                indice = i;
            }
        }
    }
    return ChooseLeaf(nodo->entradas[indice].hijo, rect);
}

MBR RTree::calcularMBR(Nodo* nodo) {
    MBR rect = nodo->entradas[0].mbr;
    for (int i = 1; i < nodo->entradas.size(); i++) {
        rect = unir(rect, nodo->entradas[i].mbr);
    }
    return rect;
}
void RTree::ajustarPadre(Nodo* nodo) {
    if (!nodo->padre) return;
    Nodo* padre = nodo->padre;
    for (int i = 0; i < padre->entradas.size(); i++) {
        if (padre->entradas[i].hijo == nodo) {
            padre->entradas[i].mbr = calcularMBR(nodo);
            return;
        }
    }
}
void RTree::AdjustTree(Nodo* nodo, Nodo* nuevo_nodo) {
    while (nodo != root) {
        Nodo* padre = nodo->padre;
        ajustarPadre(nodo);

        if (nuevo_nodo) {
            Entrada ent1;
            ent1.mbr = calcularMBR(nuevo_nodo);
            ent1.hijo = nuevo_nodo;
            nuevo_nodo->padre = padre;
            padre->entradas.push_back(ent1);
            if (padre->entradas.size() > maxEntradas) {
                Nodo* padre_nuevo = SplitNodo(padre);
                nodo = padre;
                nuevo_nodo = padre_nuevo;
            }
            else {
                nodo = padre;
                nuevo_nodo = nullptr;
            }
        }
        else {
            nodo = padre;
        }

    }
    if (nuevo_nodo) {
        Nodo* nueva_root = new Nodo(false);
        Entrada ent1, ent2;
        ent1.mbr = calcularMBR(root);
        ent2.mbr = calcularMBR(nuevo_nodo);
        ent1.hijo = root;
        ent2.hijo = nuevo_nodo;
        root->padre = nueva_root;
        nuevo_nodo->padre = nueva_root;
        nueva_root->entradas.push_back(ent1);
        nueva_root->entradas.push_back(ent2);
        root = nueva_root;
    }
}

pair<int, int> RTree::PickSeeds(Nodo* nodo) {
    int indice1{ 0 }, indice2{ 1 };
    int peor_area = numeric_limits<int>::min();
    for (int i = 0; i < nodo->entradas.size() - 1; i++) {
        for (int j = i + 1; j < nodo->entradas.size(); j++) {
            MBR rect1 = nodo->entradas[i].mbr;
            MBR rect2 = nodo->entradas[j].mbr;
            MBR rect_actual = unir(rect1, rect2);
            int area_sobrante = area(rect_actual) - area(rect1) - area(rect2);
            if (area_sobrante > peor_area) {
                peor_area = area_sobrante;
                indice1 = i;
                indice2 = j;
            }
        }
    }
    pair<int, int> semillas;
    semillas.first = indice1;
    semillas.second = indice2;
    return semillas;
}

int RTree::PickNext(MBR rect1, MBR rect2, vector<Entrada>& pendientes) {
    int indice{ 0 }; 
    int diferencia = numeric_limits<int>::min();
    for (int i = 0; i < pendientes.size(); i++) {
        MBR rect_actual = pendientes[i].mbr;
        int area_rect1 = area(unir(rect1, rect_actual)) - area(rect1);
        int area_rect2 = area(unir(rect2, rect_actual)) - area(rect2);
        int dif_actual = abs(area_rect1 - area_rect2);
        if (dif_actual > diferencia) {
            diferencia = dif_actual;
            indice = i;
        }
    }
    return indice;
}

Nodo* RTree::QuadraticSplit(Nodo* nodo) {
    pair<int, int> semillas = PickSeeds(nodo);
    vector<Entrada> grupo1, grupo2;
    MBR rect1, rect2;
    rect1 = nodo->entradas[semillas.first].mbr;
    rect2 = nodo->entradas[semillas.second].mbr;
    grupo1.push_back(nodo->entradas[semillas.first]);
    grupo2.push_back(nodo->entradas[semillas.second]);
    // Eliminar semillas (el de mayor indice primero para no desplazar el menor)
    nodo->entradas.erase(nodo->entradas.begin() + semillas.second);
    nodo->entradas.erase(nodo->entradas.begin() + semillas.first);
    while (!nodo->entradas.empty()) {
        if (grupo1.size() + nodo->entradas.size() == minEntrada) { // Metemos todas las entradas pendientes
            grupo1.insert(grupo1.end(), nodo->entradas.begin(), nodo->entradas.end());
            nodo->entradas.clear();
            break;
        }
        if (grupo2.size() + nodo->entradas.size() == minEntrada) {
            grupo2.insert(grupo2.end(), nodo->entradas.begin(), nodo->entradas.end());
            nodo->entradas.clear();
            break;
        }
        int indice = PickNext(rect1, rect2, nodo->entradas);
        MBR rect = nodo->entradas[indice].mbr;
        int area_rect1 = area(unir(rect, rect1)) - area(rect1);
        int area_rect2 = area(unir(rect, rect2)) - area(rect2);
        if (area_rect1 < area_rect2) {
            grupo1.push_back(nodo->entradas[indice]);
            rect1 = unir(rect1, rect);
        }
        else if (area_rect1 > area_rect2) {
            grupo2.push_back(nodo->entradas[indice]);
            rect2 = unir(rect2, rect);
        }
        else {
            if (area(rect1) < area(rect2)) {
                grupo1.push_back(nodo->entradas[indice]);
                rect1 = unir(rect1, rect);
            }
            else {
                grupo2.push_back(nodo->entradas[indice]);
                rect2 = unir(rect2, rect);
            }
        }
        swap(nodo->entradas[indice], nodo->entradas.back());
        nodo->entradas.pop_back();
    }
    Nodo* nuevo_nodo = new Nodo(nodo->esHoja);
    nodo->entradas = grupo1;
    nuevo_nodo->entradas = grupo2;
    //Reafirrmar padre solo si es nodo interno
    if (!nodo->esHoja) {
        for (int i = 0; i < nodo->entradas.size(); i++) {
            nodo->entradas[i].hijo->padre = nodo;
        }
        for (int i = 0; i < nuevo_nodo->entradas.size(); i++) {
            nuevo_nodo->entradas[i].hijo->padre = nuevo_nodo;
        }
    }
    return nuevo_nodo;
}

//Nodo* RTree::LinearSplit(Nodo* nodo) {}
//Nodo* RTree::ExhaustiveSplit(Nodo* nodo) {}
//Nodo* RTree::GreeneSplit(Nodo* nodo) {}

Nodo* RTree::SplitNodo(Nodo* nodo) {
    switch (tipoSplit) {
    //case LINEAR:     return LinearSplit(nodo);
    //case EXHAUSTIVE: return ExhaustiveSplit(nodo);
    //case GREENE:     return GreeneSplit(nodo);
    default:         return QuadraticSplit(nodo);
    }
}

void RTree::Insert(Punto p, MBR rect) {
    Entrada e;
    e.punto = p;
    e.mbr = rect;
    if (!root) {
        root = new Nodo(true);
        root->entradas.push_back(e);
        return;
    }
    Nodo* nodo_hoja = ChooseLeaf(root, rect);
    nodo_hoja->entradas.push_back(e);
    Nodo* nuevo_nodo = nullptr;
    if (nodo_hoja->entradas.size() > maxEntradas) {
        nuevo_nodo = SplitNodo(nodo_hoja);
    }
    AdjustTree(nodo_hoja, nuevo_nodo);
}

void RTree::Print() {
    if (!root) cout << "Arbol vacio";
    vector<pair<Nodo*, int>> pila;
    pila.push_back({root, 0});
    while (!pila.empty()) {
        Nodo* nodo = pila.back().first;
        int nivel = pila.back().second;
        pila.pop_back();
        for (int i = 0; i < nivel; i++) {
            cout << " ";
        }
        if (nodo->esHoja) {
            cout << "[HOJA] " << nodo->entradas.size() << " entradas: ";
            for (int i = 0; i < nodo->entradas.size(); i++) {
                cout << "(" << nodo->entradas[i].punto.x << "," << nodo->entradas[i].punto.y << ") ";
            }
            cout << endl;
        }
        else {
            cout << "[INTERNO] " << nodo->entradas.size() << " hijos: \n";
            for (int i = nodo->entradas.size() - 1; i >= 0; i--) {
                pila.push_back({nodo->entradas[i].hijo, nivel + 1});
            }
        }
    }
}

MBR generarMBR(int x, int y) {
    MBR rect;
    rect.xmax = x;
    rect.xmin = x;
    rect.ymax = y;
    rect.ymin = y;
    return rect;
}

int main()
{
    RTree arbol(4, QUADRATIC);
    vector<Punto> puntos = {
        {1,1},{2,3},{5,6},{7,2},{4,4},
        {8,8},{3,1},{6,5},{9,3},{2,7}
    };
    for (int i = 0; i < puntos.size(); i++) {
        arbol.Insert(puntos[i], generarMBR(puntos[i].x, puntos[i].y));
    }
    arbol.Print();
}
