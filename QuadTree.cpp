#include <iostream>
using namespace std;

struct Node {
    int x , y, size, value;
    Node* children[4];
    Node(int _x,int _y,int _size): x(_x), y(_y), size(_size), value(-1){
        for(int i = 0; i < 4; i++){
            children[i] = NULL;
        }
    }
};

class QuadTree{
    private:
    Node* root;
    bool isUniform (int** grid, int size, int x, int y){
        int val = grid[x][y];
        for(int i = x; i < x + size; i++){
            for(int j = y; j < y + size; j++){
                if(grid[i][j] != val) return 0;
            }
        }
        return 1;
    }
    Node* build (int** grid, int size, int x, int y){
        Node* node = new Node(x, y, size);
        if(isUniform(grid, size, x, y)){
            node->value = grid[x][y];
            return node;
        }
        int new_size = size / 2;
        node->children[0] = build(grid, new_size, x, y);
        node->children[1] = build(grid, new_size, x, y + new_size);
        node->children[2] = build(grid, new_size, x + new_size, y);
        node->children[3] = build(grid, new_size, x + new_size, y + new_size);
        return node;
    }
    void print(Node* node, int level){
        if(!node) return;
        if (node->value != -1) {
            cout << "Leaf: (" << node->x << "," << node->y
                 << ") size=" << node->size
                 << " val=" << node->value << endl;
        } else {
            cout << "Node: (" << node->x << "," << node->y
                 << ") size=" << node->size << endl;
        }

        print(node->children[0], level + 1);
        print(node->children[1], level + 1);
        print(node->children[2], level + 1);
        print(node->children[3], level + 1);
    }
    public:
    QuadTree(int** grid, int size){
        root = build(grid, size, 0 , 0);
    }
    void show(){
        print(root, 0);
    }
};

int main()
{
    int n = 4;
    int** grid = new int* [n];
    for(int i = 0; i < n; i++){
        grid[i] = new int [n];
    }
   int data[4][4] = {
        {1, 1, 0, 0},
        {1, 1, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0}
    };
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            grid[i][j] = data[i][j];

    QuadTree qt(grid, n);

    qt.show();

    return 0;
}
