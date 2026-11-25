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
