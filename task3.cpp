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

unsigned long long polynomialHash(string key, unsigned long long a) {
    unsigned long long hashVal = 0;
    for (int i = 0; i < key.length(); i++) {
        hashVal = a * hashVal + key[i];
    }
    return hashVal;
}
