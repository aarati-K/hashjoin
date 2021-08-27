#include <cstdlib>
#include <iostream>
#include <string>
// #include <libconfig.h++>

#include "table.h"
#include "hashjoin.h"
#include "hashjoinvip.h"

// using namespace libconfig;
using namespace std;

// Nation and Customer tables

int main(int argc, char** argv) {
    // Initialize random seed
    if (argc > 1) {
        uint seed = stoi(argv[1]);
        srandom(seed);
    }

    Table fact, dim;

    // Customer (Fact)
    fact.addCol(CT_INT, sizeof(int)); // custkey
    fact.addCol(CT_STRING, 20); // name
    fact.addCol(CT_STRING, 41); // address
    fact.addCol(CT_INT, sizeof(int)); // nationkey
    fact.addCol(CT_STRING, 16); // phone
    fact.addCol(CT_FLOAT, sizeof(float)); // accountbal
    fact.addCol(CT_STRING, 21); // mktsegment
    fact.addCol(CT_STRING, 118); // comment

    // Nation (Dimension)
    dim.addCol(CT_INT, sizeof(int)); // nationkey
    dim.addCol(CT_STRING, 15); // name
    dim.addCol(CT_INT, sizeof(int)); // regionkey
    dim.addCol(CT_STRING, 153); // comment
    
    // Load the tables
    fact.loadFromFile(string("SkewedDataGenerator/customer.tbl"), '|');
    dim.loadFromFile(string("SkewedDataGenerator/nation.tbl"), '|');

    fact.shuffle();
    dim.shuffle();
    
    Hashjoin hj;
    hj.exec(fact, 3, dim, 0);

    cout << endl;

    Hashjoinvip hjv;
    hjv.exec(fact, 3, dim, 0);
    return 0;
}
