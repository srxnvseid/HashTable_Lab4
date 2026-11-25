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

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int nextPrime(int n) {
    while (!isPrime(n)) n++;
    return n;
}

void setupTable(HashTable& ht, int size) {
    ht.table.assign(size, {"", EMPTY});
    ht.currentSize = 0;
}

unsigned long long polynomialHash(string ip) {
    unsigned long long hashVal = 0;
    for (int i = 0; i < ip.length(); i++) {
        hashVal = 37 * hashVal + ip[i];
    }
    return hashVal;
}

int getIndex(HashTable& ht, string ip) {
    unsigned long long k = polynomialHash(ip);
    int N = ht.table.size();
    return k % N;
}

int getStep(HashTable& ht, int i) {
    if (ht.mode == LINEAR) return 1;
    return 2 * i - 1;
}

double getLoadFactor(HashTable& ht) {
    return (double)ht.currentSize / ht.table.size();
}

bool insertIP(HashTable& ht, string ip);

void rehash(HashTable& ht, int newSize) {
    vector<Cell> oldTable = ht.table;
    setupTable(ht, newSize);

    for (int i = 0; i < oldTable.size(); i++) {
        if (oldTable[i].status == ACTIVE) {
            insertIP(ht, oldTable[i].ip);
        }
    }
}

bool insertIP(HashTable& ht, string ip) {
    int N = ht.table.size();
    int idx = getIndex(ht, ip);
    int firstDeleted = -1;
    int target = -1;

    for (int i = 1; i <= N; i++) {
        if (ht.table[idx].status == ACTIVE) {
            if (ht.table[idx].ip == ip) return false;
        } else if (ht.table[idx].status == DELETED) {
            if (firstDeleted == -1) firstDeleted = idx;
        } else {
            target = (firstDeleted != -1) ? firstDeleted : idx;
            break;
        }
        idx = (idx + getStep(ht, i)) % N;
    }

    if (target == -1) target = firstDeleted;

    if (target == -1) {
        rehash(ht, nextPrime(2 * N));
        return insertIP(ht, ip);
    }

    ht.table[target].ip = ip;
    ht.table[target].status = ACTIVE;
    ht.currentSize++;

    if (getLoadFactor(ht) > 0.5) {
        rehash(ht, nextPrime(2 * ht.table.size()));
    }
    return true;
}

bool containsIP(HashTable& ht, string ip, int& probesOut) {
    int N = ht.table.size();
    int idx = getIndex(ht, ip);
    probesOut = 0;

    for (int i = 1; i <= N; i++) {
        probesOut++;
        if (ht.table[idx].status == EMPTY) return false;
        if (ht.table[idx].status == ACTIVE && ht.table[idx].ip == ip) return true;
        idx = (idx + getStep(ht, i)) % N;
    }
    return false;
}

bool removeIP(HashTable& ht, string ip) {
    int N = ht.table.size();
    int idx = getIndex(ht, ip);

    for (int i = 1; i <= N; i++) {
        if (ht.table[idx].status == EMPTY) return false;
        if (ht.table[idx].status == ACTIVE && ht.table[idx].ip == ip) {
            ht.table[idx].status = DELETED;
            ht.table[idx].ip = "";
            ht.currentSize--;
            return true;
        }
        idx = (idx + getStep(ht, i)) % N;
    }
    return false;
}
