#include <cstdlib>
#include <iostream>
#include <string>
// #include <libconfig.h++>

#include "table.h"
#include "hashjoin.h"
#include "hashjoinvip.h"

// using namespace libconfig;
using namespace std;

// Supplier and PartSupplier tables

int main(int argc, char** argv) {
    // Initialize random seed
    if (argc > 1) {
        uint seed = stoi(argv[1]);
        srandom(seed);
    }

    Table fact, dim;

    // PartSupplier (Fact)
    fact.addCol(CT_INT, sizeof(int)); // partkey
    fact.addCol(CT_INT, sizeof(int)); // suppkey
    fact.addCol(CT_INT, sizeof(int)); // availqty
    fact.addCol(CT_FLOAT, sizeof(float)); // supplycost
    fact.addCol(CT_STRING, 200); // comment

    // Supplier (Dimension)
    dim.addCol(CT_INT, sizeof(int)); // suppkey
    dim.addCol(CT_STRING, 26); // name
    dim.addCol(CT_STRING, 41); // address
    dim.addCol(CT_INT, sizeof(int)); // nationkey
    dim.addCol(CT_STRING, 16); // phone
    dim.addCol(CT_FLOAT, sizeof(float)); // accountbal
    dim.addCol(CT_STRING, 102); // comment

    // Load the tables
    fact.loadFromFile(string("SkewedDataGenerator/zipf2/partsupp.tbl"), '|');
    dim.loadFromFile(string("SkewedDataGenerator/zipf2/supplier.tbl"), '|');

    fact.shuffle();
    dim.shuffle();
    
    Hashjoin hj;
    hj.exec(fact, 1, dim, 0);

    cout << endl;

    Hashjoinvip hjv;
    hjv.exec(fact, 1, dim, 0);
    return 0;
}
