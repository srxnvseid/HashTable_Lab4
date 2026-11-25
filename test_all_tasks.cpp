// ============================================================================
// test_all_tasks.cpp
//
// BU SKRIPT TEST MEQSEDILE SUNI INTELLEKT - CLAUDE SONNET 5.5 - TEREFINDEN
// YARADILIB. Komanda uzvleri skripti yazmayib; yalniz isleredib ve neticelerden
// hesabatda istifade edib.
//
// Skript komandanin ozunun task1.cpp, task2.cpp, task3.cpp fayllarini oldugu
// kimi (deyismeden) #include edir ve onlarin dogru isledyini yoxlayir:
//   Task 1: Separate Chaining + MAD + resize/shrink (std::map ile muqayise)
//   Task 2: Linear/Quadratic probing + lazy deletion + rehash
//   Task 3: achar generasiyasi, a=1 vs a=37, bütün achar-larin tapilmasi
// Performans muqayisesi (lambda sweep) ayrica task3_lambda_sweep.cpp-dedir.
//
// Her fayl oz namespace-inde (t1, t2, t3) qlobal adlarin toqqusmasinin
// qarshisini almaq ucun yuklenir. Bu fayl task1/2/3.cpp ile EYNI qovluqda olmalidir.
//
// Kompilyasiya: g++ -std=c++17 test_all_tasks.cpp -o test_all && ./test_all
// ============================================================================
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <set>
#include <map>
#include <iomanip>
#include <algorithm>

namespace t1 {
#include "task1.cpp"
}
namespace t2 {
#include "task2.cpp"
}
namespace t3 {
#include "task3.cpp"
}

using namespace std;

static int passed = 0, failed = 0;
static void check(bool ok, const string& name) {
    if (ok) { passed++; cout << "  [PASS] " << name << endl; }
    else    { failed++; cout << "  [FAIL] " << name << endl; }
}

// ---------------------------------------------------------------- Task 1
static void testTask1() {
    cout << "== Task 1: Separate Chaining + MAD + resize ==" << endl;
    for (int mad = 0; mad < 2; mad++) {
        srand(1000 + mad);
        t1::HashTable ht;
        ht.useMAD = (mad == 1);
        t1::setupTable(ht, t1::nextPrime(5));
        map<string, int> ref;
        bool opsOk = true, sizeOk = true, loadOk = true, primeOk = true;
        size_t peak = ht.table.size();
        for (int i = 0; i < 20000; i++) {
            string isbn = "isbn" + to_string(rand() % 2000);
            if (rand() % 5 < 3) {
                bool a = t1::insertBook(ht, {isbn, "t", "a", true});
                bool b = !ref.count(isbn); ref[isbn] = 1;
                if (a != b) opsOk = false;
            } else {
                bool a = t1::removeBook(ht, isbn);
                bool b = ref.erase(isbn) > 0;
                if (a != b) opsOk = false;
            }
            if ((int)ref.size() != ht.currentSize) sizeOk = false;
            if (t1::getLoadFactor(ht) >= 1.0) loadOk = false;
            if (!t1::isPrime((int)ht.table.size())) primeOk = false;
            peak = max(peak, ht.table.size());
        }
        bool findOk = true;
        for (map<string,int>::iterator it = ref.begin(); it != ref.end(); ++it) {
            t1::Book r;
            if (!t1::findByISBN(ht, it->first, r) || r.isbn != it->first) findOk = false;
        }
        string m = mad ? "MAD" : "k mod N";
        check(opsOk,   "[" + m + "] insert/remove neticeleri std::map ile eynidir (20000 emeliyyat)");
        check(sizeOk,  "[" + m + "] currentSize hemise duzgundur");
        check(loadOk,  "[" + m + "] yuk faktoru hec vaxt >= 1.0 olmur");
        check(primeOk, "[" + m + "] cedvel olcusu hemise sade ededdir");
        check(findOk,  "[" + m + "] cedvelde qalan butun ISBN-ler tapilir");
    }
    // boyume ve kichilme
    srand(7);
    t1::HashTable ht; ht.useMAD = true;
    t1::setupTable(ht, t1::nextPrime(5));
    size_t start = ht.table.size();
    for (int i = 0; i < 200; i++) t1::insertBook(ht, {"b" + to_string(i), "", "", true});
    size_t grown = ht.table.size();
    check(grown > start && t1::getLoadFactor(ht) < 1.0, "200 kitabdan sonra cedvel boyuyub (" + to_string(start) + " -> " + to_string(grown) + "), lambda < 1");
    for (int i = 0; i < 195; i++) t1::removeBook(ht, "b" + to_string(i));
    check(ht.table.size() < grown && ht.table.size() >= 11 && ht.currentSize == 5,
          "195 silmeden sonra cedvel kichilib (" + to_string(grown) + " -> " + to_string(ht.table.size()) + "), minimum 11");
    t1::Book dup = {"b199", "x", "y", true};
    check(!t1::insertBook(ht, dup), "tekrar ISBN redd edilir");
    // MAD vs k mod N (melumat ucun)
    for (int mad = 0; mad < 2; mad++) {
        srand(5);
        t1::HashTable h; h.useMAD = mad; t1::setupTable(h, 10007);
        for (int i = 0; i < 5000; i++) t1::insertBook(h, {"978-" + to_string(1000000000LL + i * 7), "", "", true});
        size_t mx = 0; for (size_t i = 0; i < h.table.size(); i++) mx = max(mx, h.table[i].size());
        cout << "  [INFO] " << (mad ? "MAD     " : "k mod N ") << " rejimi, 5000 ardicil ISBN: max zencir = " << mx << endl;
    }
}

// ---------------------------------------------------------------- Task 2
static void testTask2() {
    cout << "== Task 2: Open Addressing + Lazy Deletion + Rehash ==" << endl;
    for (int q = 0; q < 2; q++) {
        srand(2000 + q);
        t2::HashTable ht; ht.mode = q ? t2::QUADRATIC : t2::LINEAR;
        t2::setupTable(ht, t2::nextPrime(11));
        map<string, int> ref;
        bool opsOk = true, sizeOk = true, loadOk = true, containsOk = true;
        for (int i = 0; i < 20000; i++) {
            string ip = "10.0." + to_string(rand() % 40) + "." + to_string(rand() % 40);
            int r = rand() % 5;
            if (r < 3) {
                bool a = t2::insertIP(ht, ip);
                bool b = !ref.count(ip); ref[ip] = 1;
                if (a != b) opsOk = false;
                if (t2::getLoadFactor(ht) > 0.5 + 1e-9) loadOk = false;
            } else if (r == 3) {
                bool a = t2::removeIP(ht, ip);
                bool b = ref.erase(ip) > 0;
                if (a != b) opsOk = false;
            } else {
                int p; bool a = t2::containsIP(ht, ip, p);
                if (a != (ref.count(ip) > 0)) containsOk = false;
            }
            if ((int)ref.size() != ht.currentSize) sizeOk = false;
        }
        string m = q ? "QUADRATIC" : "LINEAR";
        check(opsOk,       "[" + m + "] insert/remove neticeleri std::map ile eynidir (20000 emeliyyat)");
        check(sizeOk,      "[" + m + "] currentSize hemise duzgundur");
        check(loadOk,      "[" + m + "] her insert-den sonra lambda <= 0.5");
        check(containsOk,  "[" + m + "] containsIP butun emeliyyatlarda duzgun cavab verir (silinenler daxil)");
    }
    // quadratic addim duzusu: H_i = H_0 + i^2
    {
        t2::HashTable ht; ht.mode = t2::QUADRATIC; t2::setupTable(ht, 101);
        int N = 101, idx = 17; bool ok = true;
        for (int i = 1; i <= 50; i++) {
            idx = (idx + t2::getStep(ht, i)) % N;
            if (idx != (17 + i * i) % N) ok = false;
        }
        check(ok, "Quadratic addim H_i = H_(i-1) + 2i - 1 duzusu H_0 + i^2 mod N ile eynidir (i = 1..50)");
    }
    // lazy deletion: zencir qorunur ve DELETED yeniden istifade olunur
    {
        t2::HashTable ht; ht.mode = t2::LINEAR; t2::setupTable(ht, 23);
        vector<string> same; int target = -1;
        for (int i = 0; i < 5000 && same.size() < 3; i++) {
            string ip = "172.16.0." + to_string(i);
            int idx = t2::getIndex(ht, ip);
            if (target == -1) { target = idx; same.push_back(ip); }
            else if (idx == target) same.push_back(ip);
        }
        string A = same[0], B = same[1], C = same[2];
        t2::insertIP(ht, A); t2::insertIP(ht, B);
        t2::removeIP(ht, A);
        int p = 0;
        bool foundB = t2::containsIP(ht, B, p);
        check(foundB && p >= 2, "A silindikden sonra eyni indeksli B hele de tapilir (DELETED-den kechilir, " + to_string(p) + " yoxlama)");
        check(!t2::containsIP(ht, A, p), "silinmish A artiq tapilmir");
        size_t before = ht.table.size();
        t2::insertIP(ht, C);
        int deleted = 0; for (size_t i = 0; i < ht.table.size(); i++) if (ht.table[i].status == t2::DELETED) deleted++;
        check(deleted == 0 && ht.table.size() == before, "yeni element DELETED xanani yeniden istifade edir (DELETED sayi = 0)");
    }
    // rehash DELETED-leri temizleyir
    {
        t2::HashTable ht; ht.mode = t2::QUADRATIC; t2::setupTable(ht, 23);
        for (int i = 0; i < 8; i++) t2::insertIP(ht, "1.1.1." + to_string(i));
        for (int i = 0; i < 5; i++) t2::removeIP(ht, "1.1.1." + to_string(i));
        int deletedBefore = 0; for (size_t i = 0; i < ht.table.size(); i++) if (ht.table[i].status == t2::DELETED) deletedBefore++;
        for (int i = 100; i < 140; i++) t2::insertIP(ht, "2.2.2." + to_string(i));
        int deletedAfter = 0; for (size_t i = 0; i < ht.table.size(); i++) if (ht.table[i].status == t2::DELETED) deletedAfter++;
        check(deletedBefore > 0 && deletedAfter == 0, "rehash DELETED qeydlerini temizleyir (" + to_string(deletedBefore) + " -> " + to_string(deletedAfter) + ")");
    }
}

// ---------------------------------------------------------------- Task 3
static void testTask3() {
    cout << "== Task 3: Empirik test infrastrukturu ==" << endl;
    vector<string> k1 = t3::generateKeys(5000, 42), k2 = t3::generateKeys(5000, 42);
    set<string> uniq(k1.begin(), k1.end());
    check(k1.size() == 5000 && uniq.size() == 5000, "generateKeys 5000 unikal achar yaradir");
    check(k1 == k2, "eyni seed (42) eyni achar siyahisini verir (tekrarlana bilirlik)");

    t3::SeparateChainingHash a1, a37;
    t3::setupChaining(a1, 10007, 1); t3::setupChaining(a37, 10007, 37);
    t3::OpenAddressingHash lin, quad;
    t3::setupOpen(lin, 10007, t3::LINEAR, 37); t3::setupOpen(quad, 10007, t3::QUADRATIC, 37);
    for (size_t i = 0; i < k1.size(); i++) {
        t3::insertChaining(a1, k1[i]); t3::insertChaining(a37, k1[i]);
        t3::insertOpen(lin, k1[i]); t3::insertOpen(quad, k1[i]);
    }
    check(a37.collisions < a1.collisions, "a = 37 kolliziya sayi a = 1-den azdir (" + to_string(a37.collisions) + " < " + to_string(a1.collisions) + ")");
    bool allFound = true;
    for (size_t i = 0; i < k1.size(); i++) {
        int p;
        if (!t3::containsChaining(a37, k1[i], p) || !t3::containsOpen(lin, k1[i], p) || !t3::containsOpen(quad, k1[i], p)) allFound = false;
    }
    check(allFound, "butun 5000 achar uc usulda de tapilir");
    vector<string> absent = t3::generateKeys(3000, 99);
    bool noneFound = true; int p;
    for (size_t i = 0; i < absent.size(); i++) {
        if (uniq.count(absent[i])) continue;
        if (t3::containsChaining(a37, absent[i], p) || t3::containsOpen(lin, absent[i], p) || t3::containsOpen(quad, absent[i], p)) noneFound = false;
    }
    check(noneFound, "cedvelde olmayan acharlar tapilmir (yalan musbet yoxdur)");
    check(t3::getMaxChain(a37) < t3::getMaxChain(a1), "a = 37 ile max zencir a = 1-den qisadir (" + to_string(t3::getMaxChain(a37)) + " < " + to_string(t3::getMaxChain(a1)) + ")");
}

int main() {
    testTask1();
    testTask2();
    testTask3();
    cout << endl << "NETICE: " << passed << " PASS, " << failed << " FAIL" << endl;
    return failed == 0 ? 0 : 1;
}
