#include <cstdlib>
#include <iostream>
#include <string>
// #include <libconfig.h++>

#include "table.h"
#include "hashjoin.h"
#include "hashjoinvip.h"

// using namespace libconfig;
using namespace std;

// Customer and Order tables

int main(int argc, char** argv) {
    // Initialize random seed
    if (argc > 1) {
        uint seed = stoi(argv[1]);
        srandom(seed);
    }

    Table fact, dim;

    // Order (Fact)
    fact.addCol(CT_INT, sizeof(int)); // okey
    fact.addCol(CT_INT, sizeof(int)); // custkey
    fact.addCol(CT_CHAR, sizeof(char)); // orderstatus
    fact.addCol(CT_FLOAT, sizeof(float)); // totalprice
    fact.addCol(CT_STRING, 14); // date
    fact.addCol(CT_STRING, 16); // opriority
    fact.addCol(CT_STRING, 16); // clerk
    fact.addCol(CT_INT, sizeof(int)); // shippriority
    fact.addCol(CT_STRING, 80); // comment

    // Customer (Dimension)
    dim.addCol(CT_INT, sizeof(int)); // custkey
    dim.addCol(CT_STRING, 20); // name
    dim.addCol(CT_STRING, 41); // address
    dim.addCol(CT_INT, sizeof(int)); // nationkey
    dim.addCol(CT_STRING, 16); // phone
    dim.addCol(CT_FLOAT, sizeof(float)); // accountbal
    dim.addCol(CT_STRING, 21); // mktsegment
    dim.addCol(CT_STRING, 118); // comment
    
    // Load the tables
    fact.loadFromFile(string("SkewedDataGenerator/zipf2/order.tbl"), '|');
    dim.loadFromFile(string("SkewedDataGenerator/zipf2/customer.tbl"), '|');

    for (int i=0; i<3; i++) {
    	fact.shuffle();
    	dim.shuffle();
    }
    
    Hashjoin hj;
    hj.exec(fact, 1, dim, 0);

    cout << endl;

    Hashjoinvip hjv;
    hjv.exec(fact, 1, dim, 0);
    return 0;
}
