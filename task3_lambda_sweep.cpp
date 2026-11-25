// ============================================================================
// task3_lambda_sweep.cpp
//
// BU SKRIPT TEST MEQSEDILE SUNI INTELLEKT - CLAUDE SONNET 5.5 - TEREFINDEN
// YARADILIB. Komanda uzvleri skripti yazmayib; yalniz isleredib ve neticelerden
// hesabatda istifade edib.
//
// Meqsed: Separate Chaining, Linear Probing ve Quadratic Probing usullarini
// eyni achar toplusu uzerinde avtomatik muqayise etmek ve her yuk faktoru
// (lambda) ucun hansi usulun daha yaxsi netice verdiyini mueyyen etmek.
//
// Asagidaki struktur ve funksiyalar komandanin task3.cpp faylindan oldugu kimi
// goturulub; yalniz sonda yeni main() elave olunub.
//
// "Orta yoxlama sayi" (kodda "probes"): bir axtaris zamani yoxlanilan
// element sayidir. Zencirlemede - zencirdeki achar muqayiseleri, achiq
// unvanlamada - baxilan xanalar. 1.00 = achar birbasa tapilir.
// Uqursuz axtarisda zencirleme 0 ola biler (bosh zencir = muqayise yoxdur).
//
// Kompilyasiya: g++ -std=c++17 task3_lambda_sweep.cpp -o sweep
// ============================================================================
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <set>
#include <iomanip>
#include <algorithm>

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

int getStep(OpenAddressingHash& ht, int i) {
    if (ht.mode == LINEAR) return 1;
    return 2 * i - 1;
}

bool insertOpen(OpenAddressingHash& ht, string key) {
    int N = ht.table.size();
    int idx = getOpenIndex(ht, key);
    int firstDeleted = -1;
    int target = -1;

    for (int i = 1; i <= N; i++) {
        if (ht.table[idx].status == ACTIVE) {
            if (ht.table[idx].key == key) return false;
        } else if (ht.table[idx].status == DELETED) {
            if (firstDeleted == -1) firstDeleted = idx;
        } else {
            target = (firstDeleted != -1) ? firstDeleted : idx;
            break;
        }
        idx = (idx + getStep(ht, i)) % N;
    }

    if (target == -1) target = firstDeleted;
    if (target == -1) return false;

    ht.table[target].key = key;
    ht.table[target].status = ACTIVE;
    ht.currentSize++;
    return true;
}

bool containsOpen(OpenAddressingHash& ht, string key, int& probesOut) {
    int N = ht.table.size();
    int idx = getOpenIndex(ht, key);
    probesOut = 0;

    for (int i = 1; i <= N; i++) {
        probesOut++;
        if (ht.table[idx].status == EMPTY) return false;
        if (ht.table[idx].status == ACTIVE && ht.table[idx].key == key) return true;
        idx = (idx + getStep(ht, i)) % N;
    }
    return false;
}

int getMaxCluster(OpenAddressingHash& ht) {
    int N = ht.table.size();
    int start = -1;
    for (int i = 0; i < N; i++) {
        if (ht.table[i].status != ACTIVE) {
            start = i;
            break;
        }
    }
    if (start == -1) return N;

    int maxLen = 0;
    int curLen = 0;
    for (int i = 0; i < N; i++) {
        int idx = (start + i) % N;
        if (ht.table[idx].status == ACTIVE) {
            curLen++;
            if (curLen > maxLen) maxLen = curLen;
        } else {
            curLen = 0;
        }
    }
    return maxLen;
}

double getAverageProbesOpen(OpenAddressingHash& ht, vector<string>& keys) {
    long long totalProbes = 0;
    for (int i = 0; i < keys.size(); i++) {
        int probes = 0;
        containsOpen(ht, keys[i], probes);
        totalProbes += probes;
    }
    return (double)totalProbes / keys.size();
}

vector<string> generateKeys(int count, int seed) {
    srand(seed);
    set<string> used;
    vector<string> keys;

    while ((int)keys.size() < count) {
        string isbn = "978-";
        for (int i = 0; i < 10; i++) {
            isbn += (char)('0' + rand() % 10);
        }
        if (used.insert(isbn).second) {
            keys.push_back(isbn);
        }
    }
    return keys;
}


// ----------------------------------------------------------------------------
// Avtomatlasdirilmis test hissesi (SI terefinden yazilib)
// ----------------------------------------------------------------------------
static int countNonEmpty(SeparateChainingHash& h) {
    int c = 0;
    for (int i = 0; i < (int)h.table.size(); i++) if (!h.table[i].empty()) c++;
    return c;
}

static const char* NAMES[3] = {"Separate Chaining", "Linear Probing", "Quadratic Probing"};

static int bestIndex(const double v[3]) {
    int b = 0;
    for (int i = 1; i < 3; i++) if (v[i] < v[b]) b = i;
    return b;
}

int main() {
    const int tableSize = 10007;
    const int seed = 42;
    const double lambdas[] = {0.25, 0.50, 0.75, 0.90};

    cout << fixed << setprecision(4);

    // ---- 1) a = 1 ve a = 37 muqayisesi (5000 achar) -----------------------
    {
        vector<string> keys = generateKeys(5000, seed);
        SeparateChainingHash h1, h37;
        setupChaining(h1, tableSize, 1);
        setupChaining(h37, tableSize, 37);
        for (int i = 0; i < (int)keys.size(); i++) {
            insertChaining(h1, keys[i]);
            insertChaining(h37, keys[i]);
        }
        cout << "== 1) Polinom sabiti: a = 1 vs a = 37 (5000 achar, N = " << tableSize << ") ==" << endl;
        cout << "a = 1 : kolliziya=" << h1.collisions << " maxZencir=" << getMaxChain(h1)
             << " orta yoxlama=" << getAverageProbesChaining(h1, keys)
             << " bosh olmayan xana=" << countNonEmpty(h1) << endl;
        cout << "a = 37: kolliziya=" << h37.collisions << " maxZencir=" << getMaxChain(h37)
             << " orta yoxlama=" << getAverageProbesChaining(h37, keys)
             << " bosh olmayan xana=" << countNonEmpty(h37) << endl;
        cout << "Qalib: " << (getAverageProbesChaining(h37, keys) < getAverageProbesChaining(h1, keys) ? "a = 37" : "a = 1") << endl << endl;
    }

    // ---- 2) lambda-ya gore avtomatik test ---------------------------------
    cout << "== 2) Yuk faktoru lambda-nin tesiri (N = " << tableSize << ", a = 37, seed = " << seed << ") ==" << endl;
    cout << "(Orta yoxlama sayi; uqurlu = achar cedvelde var, uqursuz = achar cedvelde yoxdur)" << endl << endl;

    int winsHit[3] = {0, 0, 0};
    int winsMiss[3] = {0, 0, 0};
    int openWinsHit[3] = {0, 0, 0};
    int openWinsMiss[3] = {0, 0, 0};

    for (int li = 0; li < 4; li++) {
        double lam = lambdas[li];
        int n = (int)(lam * tableSize + 0.5);
        vector<string> keys = generateKeys(n, seed);

        // cedvelde olmayan 3000 achar (uqursuz axtaris ucun)
        set<string> inTable(keys.begin(), keys.end());
        vector<string> missing;
        vector<string> pool = generateKeys(6000, 99);
        for (int i = 0; i < (int)pool.size() && (int)missing.size() < 3000; i++) {
            if (!inTable.count(pool[i])) missing.push_back(pool[i]);
        }

        SeparateChainingHash sc;
        OpenAddressingHash lin, quad;
        setupChaining(sc, tableSize, 37);
        setupOpen(lin, tableSize, LINEAR, 37);
        setupOpen(quad, tableSize, QUADRATIC, 37);

        vector<string> okLin, okQuad;
        int failLin = 0, failQuad = 0;
        for (int i = 0; i < (int)keys.size(); i++) {
            insertChaining(sc, keys[i]);
            if (insertOpen(lin, keys[i])) okLin.push_back(keys[i]); else failLin++;
            if (insertOpen(quad, keys[i])) okQuad.push_back(keys[i]); else failQuad++;
        }

        double hit[3], miss[3];
        hit[0] = getAverageProbesChaining(sc, keys);
        hit[1] = getAverageProbesOpen(lin, okLin);
        hit[2] = getAverageProbesOpen(quad, okQuad);
        miss[0] = getAverageProbesChaining(sc, missing);
        miss[1] = getAverageProbesOpen(lin, missing);
        miss[2] = getAverageProbesOpen(quad, missing);

        cout << "lambda = " << lam << "  (achar sayi n = " << n << ")" << endl;
        cout << "  " << left << setw(20) << "Usul" << right << setw(14) << "uqurlu" << setw(14) << "uqursuz"
             << setw(20) << "max zencir/klaster" << setw(12) << "yerlesmeyen" << endl;
        cout << "  " << left << setw(20) << NAMES[0] << right << setw(14) << hit[0] << setw(14) << miss[0]
             << setw(20) << getMaxChain(sc) << setw(12) << 0 << endl;
        cout << "  " << left << setw(20) << NAMES[1] << right << setw(14) << hit[1] << setw(14) << miss[1]
             << setw(20) << getMaxCluster(lin) << setw(12) << failLin << endl;
        cout << "  " << left << setw(20) << NAMES[2] << right << setw(14) << hit[2] << setw(14) << miss[2]
             << setw(20) << getMaxCluster(quad) << setw(12) << failQuad << endl;

        int bh = bestIndex(hit), bm = bestIndex(miss);
        double hitOpen[3] = {1e18, hit[1], hit[2]};
        double missOpen[3] = {1e18, miss[1], miss[2]};
        int boh = bestIndex(hitOpen), bom = bestIndex(missOpen);
        winsHit[bh]++; winsMiss[bm]++; openWinsHit[boh]++; openWinsMiss[bom]++;

        cout << "  Qalib (uqurlu): " << NAMES[bh] << " | Qalib (uqursuz): " << NAMES[bm] << endl;
        cout << "  Achiq unvanlama daxilinde qalib (uqurlu / uqursuz): " << NAMES[boh] << " / " << NAMES[bom] << endl << endl;
    }

    cout << "== 3) Yekun (4 lambda qiymeti uzre qalibiyyet sayi) ==" << endl;
    for (int i = 0; i < 3; i++) {
        cout << left << setw(20) << NAMES[i] << right
             << " uqurlu: " << winsHit[i] << "/4, uqursuz: " << winsMiss[i] << "/4" << endl;
    }
    cout << "Achiq unvanlama daxilinde: Linear (uqurlu " << openWinsHit[1] << "/4, uqursuz " << openWinsMiss[1]
         << "/4), Quadratic (uqurlu " << openWinsHit[2] << "/4, uqursuz " << openWinsMiss[2] << "/4)" << endl;
    return 0;
}
