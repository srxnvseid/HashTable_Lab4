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

void setupChaining(SeparateChainingHash& ht, int size, unsigned long long a) {
    ht.table.assign(size, vector<string>());
    ht.currentSize = 0;
    ht.a = a;
    ht.collisions = 0;
}

int getChainingIndex(SeparateChainingHash& ht, string key) {
    unsigned long long k = polynomialHash(key, ht.a);
    int N = ht.table.size();
    return k % N;
}

bool insertChaining(SeparateChainingHash& ht, string key) {
    int idx = getChainingIndex(ht, key);

    for (int i = 0; i < ht.table[idx].size(); i++) {
        if (ht.table[idx][i] == key) return false;
    }

    if (!ht.table[idx].empty()) {
        ht.collisions++;
    }

    ht.table[idx].push_back(key);
    ht.currentSize++;
    return true;
}

bool containsChaining(SeparateChainingHash& ht, string key, int& probesOut) {
    int idx = getChainingIndex(ht, key);
    probesOut = 0;

    for (int i = 0; i < ht.table[idx].size(); i++) {
        probesOut++;
        if (ht.table[idx][i] == key) return true;
    }
    return false;
}

int getMaxChain(SeparateChainingHash& ht) {
    int maxLen = 0;
    for (int i = 0; i < ht.table.size(); i++) {
        if ((int)ht.table[i].size() > maxLen) {
            maxLen = ht.table[i].size();
        }
    }
    return maxLen;
}

double getAverageProbesChaining(SeparateChainingHash& ht, vector<string>& keys) {
    long long totalProbes = 0;
    for (int i = 0; i < keys.size(); i++) {
        int probes = 0;
        containsChaining(ht, keys[i], probes);
        totalProbes += probes;
    }
    return (double)totalProbes / keys.size();
}

void setupOpen(OpenAddressingHash& ht, int size, ProbeMode mode, unsigned long long a) {
    ht.table.assign(size, {"", EMPTY});
    ht.currentSize = 0;
    ht.mode = mode;
    ht.a = a;
}

int getOpenIndex(OpenAddressingHash& ht, string key) {
    unsigned long long k = polynomialHash(key, ht.a);
    int N = ht.table.size();
    return k % N;
}
