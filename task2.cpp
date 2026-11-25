#include <iostream>
#include <vector>
#include <string>

using namespace std;

enum Status { EMPTY, ACTIVE, DELETED };
enum ProbeMode { LINEAR, QUADRATIC };

struct Cell {
    string ip;
    Status status;
};

struct HashTable {
    vector<Cell> table;
    int currentSize;
    ProbeMode mode;
};
