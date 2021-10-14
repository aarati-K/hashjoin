#include <cstdlib>
#include <iostream>

#include "table.h"
#include "hashjoin.h"
#include "hashjoinvip.h"

using namespace std;

int main(int argc, char** argv) {
    // Initialize random seed
    if (argc > 1) {
        uint seed = stoi(argv[1]);
        srandom(seed);
    }

    Table fact, dim;
    fact.addCol(CT_LONG, sizeof(ulong));
    fact.addCol(CT_LONG, sizeof(ulong));
    dim.addCol(CT_LONG, sizeof(ulong));
    dim.addCol(CT_LONG, sizeof(ulong));

    // Load the tables
    fact.loadFromFile(string("data/fact.tbl"), '|');
    dim.loadFromFile(string("data/dim.tbl"), '|');

    fact.shuffle();
    dim.shuffle();
    
    Hashjoin hj;
    hj.exec(fact, 0, dim, 0);

    cout << endl;

    Hashjoinvip hjv;
    hjv.exec(fact, 0, dim, 0);
    return 0;
}
