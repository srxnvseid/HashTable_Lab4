#include <iostream>
#include <vector>
#include <string>

using namespace std;

enum Status { EMPTY, ACTIVE, DELETED }; // her xananin 3 veziyyeti
enum ProbeMode { LINEAR, QUADRATIC }; // kolliziya zamani novbeti xananin nece axtarilacagi

struct Cell {
    string ip;
    Status status;
};

struct HashTable { // open addressing hashtable strukturu
    vector<Cell> table; // butun melumatlar tek bitisik massivde saxlanilir (cache ucun)
    int currentSize; // yalniz ACTIVE elementlerin sayi
    ProbeMode mode; // LINEAR ve ya QUADRATIC
};

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int nextPrime(int n) { // cedvel olcusu sade eded olmalidir ki quadratic probing butun cedveli gezsin
    while (!isPrime(n)) n++;
    return n;
}

void setupTable(HashTable& ht, int size) {
    ht.table.assign(size, {"", EMPTY}); // yeni bos cedvel yaradir
    ht.currentSize = 0;
}

unsigned long long polynomialHash(string ip) { // ip setrini boyuk bir edede cevirir 
    unsigned long long hashVal = 0;
    for (int i = 0; i < ip.length(); i++) {
        hashVal = 37 * hashVal + ip[i]; // 37 polinom sabitidir
    }
    return hashVal;
}

int getIndex(HashTable& ht, string ip) { // baslangic xanani (H0) mueyyen edir
    unsigned long long k = polynomialHash(ip);
    int N = ht.table.size();
    return k % N;
}

int getStep(HashTable& ht, int i) { // novbeti xanaya nece addim atilacaq
    if (ht.mode == LINEAR) return 1; // linear: her defe 1 xana ireli
    return 2 * i - 1; // quadratic: H(i) = H(i-1) + 2i - 1, cunki i^2 - (i-1)^2 = 2i - 1
}

double getLoadFactor(HashTable& ht) { // lambda ededinin mueyyen edilmesi
    return (double)ht.currentSize / ht.table.size();
}

bool insertIP(HashTable& ht, string ip);

void rehash(HashTable& ht, int newSize) { // cedvel cox dolanda melumatlari muveqqeti saxlayib
    vector<Cell> oldTable = ht.table;     // yeni boyuk cedvele yeniden elave etmek
    setupTable(ht, newSize);

    for (int i = 0; i < oldTable.size(); i++) {
        if (oldTable[i].status == ACTIVE) { // yalniz ACTIVE-ler kocurulur, DELETED-ler temizlenir
            insertIP(ht, oldTable[i].ip); // N deyisdiyi ucun yeri yeniden hesablanir
        }
    }
}

bool insertIP(HashTable& ht, string ip) {
    int N = ht.table.size();
    int idx = getIndex(ht, ip);
    int firstDeleted = -1; // rast gelinen ilk DELETED xana (-1 = hele yoxdur)
    int target = -1; // ip-nin yazilacagi xana (-1 = hele bilinmir)

    for (int i = 1; i <= N; i++) { // en cox N addim, sonsuz dovre olmasin deye
        if (ht.table[idx].status == ACTIVE) {
            if (ht.table[idx].ip == ip) return false; // dublikat, artiq qara siyahidadir
        } else if (ht.table[idx].status == DELETED) {
            if (firstDeleted == -1) firstDeleted = idx; // yeri yadda saxla, amma yazma: ip irelide ola biler
        } else { // EMPTY: zencirin sonu, demeli ip cedvelde yoxdur
            target = (firstDeleted != -1) ? firstDeleted : idx; // DELETED varsa ora, yoxsa bu bos xanaya
            break;
        }
        idx = (idx + getStep(ht, i)) % N; // novbeti xana, % N sondan basa qaytarir
    }

    if (target == -1) target = firstDeleted; // EMPTY tapilmadisa DELETED xanadan istifade et

    if (target == -1) { // hec yer tapilmadi: cedveli boyut ve yeniden cehd et
        rehash(ht, nextPrime(2 * N));
        return insertIP(ht, ip);
    }

    ht.table[target].ip = ip;
    ht.table[target].status = ACTIVE;
    ht.currentSize++;

    if (getLoadFactor(ht) > 0.5) {     // open addressing de lambda 0.5 den cox olarsa performans keskin dusur
        rehash(ht, nextPrime(2 * ht.table.size()));     // ve quadratic bos yer tapmaya biler
    }
    return true;
}

bool containsIP(HashTable& ht, string ip, int& probesOut) { // probesOut referansdir, nece xanaya baxildigini cagirana qaytarir
    int N = ht.table.size();
    int idx = getIndex(ht, ip);
    probesOut = 0;

    for (int i = 1; i <= N; i++) {
        probesOut++; // her baxilan xana 1 probe
        if (ht.table[idx].status == EMPTY) return false; // zencir bitdi, ip yoxdur
        if (ht.table[idx].status == ACTIVE && ht.table[idx].ip == ip) return true;
        idx = (idx + getStep(ht, i)) % N; // DELETED xanada dayanmiriq, ustunden kecirik
    }
    return false;
}

bool removeIP(HashTable& ht, string ip) {
    int N = ht.table.size();
    int idx = getIndex(ht, ip);

    for (int i = 1; i <= N; i++) {
        if (ht.table[idx].status == EMPTY) return false; // ip tapilmadi, silinecek bir sey yoxdur
        if (ht.table[idx].status == ACTIVE && ht.table[idx].ip == ip) {
            ht.table[idx].status = DELETED; // lazy deletion: EMPTY yox DELETED, yoxsa axtaris zenciri qirilar
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
    firewall.mode = QUADRATIC;      // probing usulu
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
