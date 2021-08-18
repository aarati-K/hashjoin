#include <cstdlib>
#include <iostream>
#include <string>
// #include <libconfig.h++>

#include "table.h"
#include "hashjoin.h"
#include "hashjoinvip.h"

// using namespace libconfig;
using namespace std;

int main() {
    Table fact, dim;
    fact.addCol(CT_INT, sizeof(int));
    fact.addCol(CT_INT, sizeof(int));
    dim.addCol(CT_INT, sizeof(int));
    dim.addCol(CT_INT, sizeof(int));

    // Load the tables
    fact.loadFromFile(string("data/fact.tbl"), '|');
    dim.loadFromFile(string("data/dim.tbl"), '|');

    fact.shuffle();
    dim.shuffle();
    
    Hashjoin hj;
    hj.exec(fact, 0, dim, 0);

    Hashjoinvip hjv;
    hjv.exec(fact, 0, dim, 0);
    return 0;
}
