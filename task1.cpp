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
