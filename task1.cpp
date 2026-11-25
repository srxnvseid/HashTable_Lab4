#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

struct Book {
    string isbn;
    string title;
    string author;
    bool isAvailable;
};

struct HashTable {
    vector<vector<Book>> table;
    int currentSize;
    bool useMAD;
    unsigned long long p, a, b;
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
    ht.table.assign(size, vector<Book>());
    ht.currentSize = 0;
    ht.p = nextPrime(size + 1);
    ht.a = 1 + rand() % (ht.p - 1);
    ht.b = rand() % ht.p;
}

unsigned long long polynomialHash(string isbn) {
    unsigned long long hashVal = 0;
    for (int i = 0; i < isbn.length(); i++) {
        hashVal = 37 * hashVal + isbn[i];
    }
    return hashVal;
}

int getIndex(HashTable& ht, string isbn) {
    unsigned long long k = polynomialHash(isbn);
    int N = ht.table.size();
    if (!ht.useMAD) return k % N;
    return ((ht.a * (k % ht.p) + ht.b) % ht.p) % N;
}

double getLoadFactor(HashTable& ht) {
    return (double)ht.currentSize / ht.table.size();
}

bool insertBook(HashTable& ht, Book b);

void rehash(HashTable& ht, int newSize) {
    vector<vector<Book>> oldTable = ht.table;
    setupTable(ht, newSize);

    for (int i = 0; i < oldTable.size(); i++) {
        for (int j = 0; j < oldTable[i].size(); j++) {
            insertBook(ht, oldTable[i][j]);
        }
    }
}

bool insertBook(HashTable& ht, Book b) {
    int idx = getIndex(ht, b.isbn);

    for (int i = 0; i < ht.table[idx].size(); i++) {
        if (ht.table[idx][i].isbn == b.isbn) return false;
    }

    ht.table[idx].push_back(b);
    ht.currentSize++;

    if (getLoadFactor(ht) >= 1.0) {
        rehash(ht, nextPrime(2 * ht.table.size()));
    }
    return true;
}

bool findByISBN(HashTable& ht, string isbn, Book& result) {
    int idx = getIndex(ht, isbn);

    for (int i = 0; i < ht.table[idx].size(); i++) {
        if (ht.table[idx][i].isbn == isbn) {
            result = ht.table[idx][i];
            return true;
        }
    }
    return false;
}

bool removeBook(HashTable& ht, string isbn) {
    int idx = getIndex(ht, isbn);

    for (int i = 0; i < ht.table[idx].size(); i++) {
        if (ht.table[idx][i].isbn == isbn) {
            ht.table[idx].erase(ht.table[idx].begin() + i);
            ht.currentSize--;

            if (ht.table.size() > 11 && getLoadFactor(ht) < 0.25) {
                int newSize = nextPrime(ht.table.size() / 2);
                if (newSize >= 11) rehash(ht, newSize);
            }
            return true;
        }
    }
    return false;
}

int main() {
    HashTable library;
    library.useMAD = true;
    setupTable(library, nextPrime(5));

    cout << "Baslangic olcu: " << library.table.size() << endl;

    insertBook(library, {"111", "Clean Code", "Robert Martin", true});
    insertBook(library, {"222", "Design Patterns", "Erich Gamma", true});
    insertBook(library, {"333", "C Programming", "Dennis Ritchie", true});
    insertBook(library, {"444", "C++ Programming", "Bjarne Stroustrup", false});
    insertBook(library, {"555", "Algorithms", "Thomas Cormen", true});

    cout << "5 kitabdan sonra olcu: " << library.table.size() << endl;

    Book found;
    if (findByISBN(library, "333", found)) {
        cout << "Tapilan kitab: " << found.title << endl;
    } else {
        cout << "Kitab tapilmadi." << endl;
    }

    removeBook(library, "333");
    cout << "Silindikden sonra say: " << library.currentSize << endl;

    return 0;
}
