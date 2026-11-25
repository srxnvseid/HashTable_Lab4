#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <set>
#include <iomanip>

using namespace std;

enum Status { EMPTY, ACTIVE, DELETED };
enum ProbeMode { LINEAR, QUADRATIC };

struct Cell {
    string key;
    Status status;
};

struct SeparateChainingHash {
    vector<vector<string>> table;
    int currentSize;
    unsigned long long a;
    int collisions;
};

struct OpenAddressingHash {
    vector<Cell> table;
    int currentSize;
    ProbeMode mode;
    unsigned long long a;
};
