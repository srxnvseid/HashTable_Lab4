#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <set>
#include <iomanip>

using namespace std;

// Open Addressing ucun xananin veziyyetleri: bos, aktiv, silinmis (lazy deletion)
enum Status { EMPTY, ACTIVE, DELETED };

// Aciq unvanlama (Open Addressing) ucun axtaris rejimi
enum ProbeMode { LINEAR, QUADRATIC };

// Open Addressing cedvelindeki tek xana strukturu
struct Cell {
    string key;    // saxlanilan acar (ISBN)
    Status status; // xananin cari veziyyeti
};

// Separate Chaining (zencirleme) hash cedveli strukturu
struct SeparateChainingHash {
    vector<vector<string>> table; // her indeksde toqqusan acarlari saxlayan zencir (vektor)
    int currentSize;              // cedveldeki cari element sayi
    unsigned long long a;         // polinom hash funksiyasinin sabiti (meselen, 1 ve ya 37)
    int collisions;               // yerlesdirme zamani yaranan umumi kolliziya sayi
};

// Open Addressing (Linear / Quadratic Probing) hash cedveli strukturu
struct OpenAddressingHash {
    vector<Cell> table;   // birbasa acarlari saxlayan xanalar massivi
    int currentSize;      // cedveldeki aktiv elementlerin sayi
    ProbeMode mode;       // LINEAR ve ya QUADRATIC addim rejimi
    unsigned long long a; // polinom hash funksiyasinin sabiti
};

// Polinom hash funksiyasi: h = a * h + key[i]
// a = 1 olduqda sade ASCII cemi verir, a = 37 olduqda acarlari daha beraber paylayir
unsigned long long polynomialHash(string key, unsigned long long a) {
    unsigned long long hashVal = 0;
    for (int i = 0; i < key.length(); i++) {
        hashVal = a * hashVal + key[i];
    }
    return hashVal;
}

// Separate Chaining cedvelini verilen olcu ve a sabiti ile baslangic veziyyete getirir
void setupChaining(SeparateChainingHash& ht, int size, unsigned long long a) {
    ht.table.assign(size, vector<string>());
    ht.currentSize = 0;
    ht.a = a;
    ht.collisions = 0;
}

// Separate Chaining ucun acarin cedvel indeksini hesablayir (k mod N)
int getChainingIndex(SeparateChainingHash& ht, string key) {
    unsigned long long k = polynomialHash(key, ht.a);
    int N = ht.table.size();
    return k % N;
}

// Separate Chaining cedveline acar elave edir ve kolliziya bas vererse saygaci artirir
bool insertChaining(SeparateChainingHash& ht, string key) {
    int idx = getChainingIndex(ht, key);

    // tekrar acarin qarsisini almaq ucun zenciri yoxlayir
    for (int i = 0; i < ht.table[idx].size(); i++) {
        if (ht.table[idx][i] == key) return false;
    }

    // eger bu indeksde artiq en azi 1 element varsa, kolliziya qeyde alinir
    if (!ht.table[idx].empty()) {
        ht.collisions++;
    }

    ht.table[idx].push_back(key);
    ht.currentSize++;
    return true;
}

// Separate Chaining cedvelinde acari axtarir ve edilen yoxlama (probe) sayini probesOut-a yazir
bool containsChaining(SeparateChainingHash& ht, string key, int& probesOut) {
    int idx = getChainingIndex(ht, key);
    probesOut = 0;

    for (int i = 0; i < ht.table[idx].size(); i++) {
        probesOut++; // zencirde her muqayise bir probe sayilir
        if (ht.table[idx][i] == key) return true;
    }
    return false;
}

// Separate Chaining cedvelindeki en uzun zencirin uzunlugunu tapir
int getMaxChain(SeparateChainingHash& ht) {
    int maxLen = 0;
    for (int i = 0; i < ht.table.size(); i++) {
        if ((int)ht.table[i].size() > maxLen) {
            maxLen = ht.table[i].size();
        }
    }
    return maxLen;
}

// Verilen butun acarlari Separate Chaining cedvelinde axtararaq orta probe sayini hesablayir
double getAverageProbesChaining(SeparateChainingHash& ht, vector<string>& keys) {
    long long totalProbes = 0;
    for (int i = 0; i < keys.size(); i++) {
        int probes = 0;
        containsChaining(ht, keys[i], probes);
        totalProbes += probes;
    }
    return (double)totalProbes / keys.size();
}

// Open Addressing cedvelini verilen olcu, rejim ve a sabiti ile baslangic veziyyete getirir
void setupOpen(OpenAddressingHash& ht, int size, ProbeMode mode, unsigned long long a) {
    ht.table.assign(size, {"", EMPTY});
    ht.currentSize = 0;
    ht.mode = mode;
    ht.a = a;
}

// Open Addressing ucun acarin baslangic indeksini hesablayir (k mod N)
int getOpenIndex(OpenAddressingHash& ht, string key) {
    unsigned long long k = polynomialHash(key, ht.a);
    int N = ht.table.size();
    return k % N;
}

// Novbeti xanaya kecid addimini qaytarir:
// LINEAR -> +1, QUADRATIC -> +(2*i - 1) (umumi olaraq H_0 + i^2 mod N verir)
int getStep(OpenAddressingHash& ht, int i) {
    if (ht.mode == LINEAR) return 1;
    return 2 * i - 1;
}

// Open Addressing cedveline sabit olcude (rehash etmeden) acar elave edir
bool insertOpen(OpenAddressingHash& ht, string key) {
    int N = ht.table.size();
    int idx = getOpenIndex(ht, key);
    int firstDeleted = -1; // rast gelinen ilk DELETED xananin indeksi
    int target = -1;       // acarin yazilacagi hedef indeks

    for (int i = 1; i <= N; i++) {
        if (ht.table[idx].status == ACTIVE) {
            if (ht.table[idx].key == key) return false; // tekrar acar redd edilir
        } else if (ht.table[idx].status == DELETED) {
            if (firstDeleted == -1) firstDeleted = idx; // ilk silinmis xanani yadda saxlayir
        } else {
            // bos xana tapildi: evvel DELETED gorulubse ora, eks halda bu xanaya yazilacaq
            target = (firstDeleted != -1) ? firstDeleted : idx;
            break;
        }
        idx = (idx + getStep(ht, i)) % N;
    }

    if (target == -1) target = firstDeleted;
    if (target == -1) return false; // yer tapilmadi

    ht.table[target].key = key;
    ht.table[target].status = ACTIVE;
    ht.currentSize++;
    return true;
}

// Open Addressing cedvelinde acari axtarir ve yoxlanilan xana (probe) sayini probesOut-a yazir
bool containsOpen(OpenAddressingHash& ht, string key, int& probesOut) {
    int N = ht.table.size();
    int idx = getOpenIndex(ht, key);
    probesOut = 0;

    for (int i = 1; i <= N; i++) {
        probesOut++; // baxilan her xana bir probe sayilir
        if (ht.table[idx].status == EMPTY) return false; // bos xana demekdir ki, acar yoxdur
        if (ht.table[idx].status == ACTIVE && ht.table[idx].key == key) return true;
        idx = (idx + getStep(ht, i)) % N;
    }
    return false;
}

// Open Addressing cedvelinde ardicil dolu (ACTIVE) xanalarin maksimum klaster uzunlugunu tapir
int getMaxCluster(OpenAddressingHash& ht) {
    int N = ht.table.size();
    int start = -1;

    // dairevi (wrap-around) klasteri duzgun olcmek ucun aktiv olmayan ilk xanadan baslayiriq
    for (int i = 0; i < N; i++) {
        if (ht.table[i].status != ACTIVE) {
            start = i;
            break;
        }
    }
    if (start == -1) return N; // butun xanalar doludur

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

// Verilen butun acarlari Open Addressing cedvelinde axtararaq orta probe sayini hesablayir
double getAverageProbesOpen(OpenAddressingHash& ht, vector<string>& keys) {
    long long totalProbes = 0;
    for (int i = 0; i < keys.size(); i++) {
        int probes = 0;
        containsOpen(ht, keys[i], probes);
        totalProbes += probes;
    }
    return (double)totalProbes / keys.size();
}

// Sabit seed ile deterministik ve unikal ISBN acarlari generasiya edir
vector<string> generateKeys(int count, int seed) {
    srand(seed);      // eyni seed her ise salmada eyni acarlari verir
    set<string> used; // unikal olmagi temin edir
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

int main() {
    int keyCount = 5000;   // generasiya olunacaq unikal acar sayi
    int tableSize = 10007; // sabit sade cedvel olcusu
    int seed = 42;         // tekrarlana bilen netice ucun sabit seed

    vector<string> keys = generateKeys(keyCount, seed);

    // 1-ci hisse: a = 1 ve a = 37 polinom sabitlerinin muqayisesi ucun cedveller
    SeparateChainingHash scA1;
    SeparateChainingHash scA37;
    setupChaining(scA1, tableSize, 1);
    setupChaining(scA37, tableSize, 37);

    // 2-ci hisse: Linear ve Quadratic Probing muqayisesi ucun cedveller (a = 37)
    OpenAddressingHash linearHT;
    OpenAddressingHash quadraticHT;
    setupOpen(linearHT, tableSize, LINEAR, 37);
    setupOpen(quadraticHT, tableSize, QUADRATIC, 37);

    // Butun 5000 acari eyni anda her dord cedvele elave edirik
    for (int i = 0; i < keys.size(); i++) {
        insertChaining(scA1, keys[i]);
        insertChaining(scA37, keys[i]);
        insertOpen(linearHT, keys[i]);
        insertOpen(quadraticHT, keys[i]);
    }

    // Polinom sabiti muqayisesinin neticeleri
    cout << "Polinom Sabiti Muqayisesi (Acar sayi: " << keyCount
         << ", TableSize: " << tableSize << ", Seed: " << seed << ")" << endl;
    cout << "a = 1 (ASCII cemi) umumi kolliziya sayi: " << scA1.collisions << endl;
    cout << "a = 37 umumi kolliziya sayi: " << scA37.collisions << endl;

    // Uc kolliziya helli usulunun muqayisesi (max zencir/klaster ve orta probe)
    cout << endl;
    cout << fixed << setprecision(4);
    cout << "Usullarin Muqayisesi (TableSize: " << tableSize << ", a = 37)" << endl;

    cout << "Separate Chaining -> Maksimum zencir uzunlugu: " << getMaxChain(scA37)
         << ", Orta axtaris addimi: " << getAverageProbesChaining(scA37, keys) << endl;

    cout << "Linear Probing    -> Maksimum klaster uzunlugu: " << getMaxCluster(linearHT)
         << ", Orta axtaris addimi: " << getAverageProbesOpen(linearHT, keys) << endl;

    cout << "Quadratic Probing -> Maksimum klaster uzunlugu: " << getMaxCluster(quadraticHT)
         << ", Orta axtaris addimi: " << getAverageProbesOpen(quadraticHT, keys) << endl;

    return 0;
}
