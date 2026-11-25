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

int main() {
    HashTable firewall;
    firewall.mode = QUADRATIC;
    setupTable(firewall, nextPrime(11));

    cout << "Baslangic olcu: " << firewall.table.size() << endl;

    insertIP(firewall, "192.168.1.10");
    insertIP(firewall, "10.0.0.5");
    insertIP(firewall, "172.16.0.1");
    insertIP(firewall, "8.8.8.8");
    insertIP(firewall, "45.33.32.156");

    cout << "5 IP-den sonra olcu: " << firewall.table.size()
         << ", yuk faktoru: " << getLoadFactor(firewall) << endl;

    insertIP(firewall, "203.0.113.7");
    cout << "6 IP-den sonra olcu (rehash): " << firewall.table.size()
         << ", yuk faktoru: " << getLoadFactor(firewall) << endl;

    if (!insertIP(firewall, "8.8.8.8")) {
        cout << "8.8.8.8 artiq qara siyahidadir." << endl;
    }

    int probes;
    if (containsIP(firewall, "172.16.0.1", probes)) {
        cout << "172.16.0.1 bloklanib (" << probes << " probe)" << endl;
    } else {
        cout << "172.16.0.1 tapilmadi." << endl;
    }

    removeIP(firewall, "172.16.0.1");
    cout << "Silindikden sonra say: " << firewall.currentSize << endl;

    if (containsIP(firewall, "172.16.0.1", probes)) {
        cout << "172.16.0.1 hele de bloklanib." << endl;
    } else {
        cout << "172.16.0.1 blokdan cixarildi (" << probes << " probe)" << endl;
    }

    return 0;
}
