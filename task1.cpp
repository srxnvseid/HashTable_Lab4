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
