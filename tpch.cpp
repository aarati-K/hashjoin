#include <cstdlib>
#include <iostream>
#include <string>
// #include <libconfig.h++>

#include "table.h"

// using namespace libconfig;
using namespace std;

int main() {
    Table t;

    // Initialize TPC-H order table schema
    ColumnType ct[] = {CT_INT,      // orderkey
                        CT_INT,     // custkey
                        CT_CHAR,    // orderstatus
                        CT_FLOAT,   // totalprice
                        CT_STRING,  // orderdate
                        CT_STRING,  // order-priority
                        CT_STRING,  // clerk
                        CT_INT,     // ship-priority
                        CT_STRING   // comment
                        };
    int sizes[] = { sizeof(int),    // CT_INT, orderkey
                    sizeof(int),    // CT_INT, custkey
                    sizeof(char),    // CT_CHAR, orderstatus
                    sizeof(float),    // CT_FLOAT, totalprice
                    11,    // CT_STRING, orderdate
                    10,    // CT_STRING, order-priority
                    16,    // CT_STRING, clerk
                    sizeof(int),    // CT_INT, ship-priority
                    67    // CT_STRING, comment
                        };
    int len = 9;
    for (int i=0; i < len; i++) {
        t.addCol(ct[i], sizes[i]);
    }
    cout << "Num cols: " << t.numCols() << endl;
    // t.printCols();
    // t.printColOffsets();
    string fname("data/bkup_order.tbl");
    t.loadFromFile(fname, '|');
    t.shuffle();

    // iterate over the tuples
    cout << "Iterating over first column" << endl;
    ColumnInfo c = t.getColumnInfo(0);
    void *addr = c.startAddr;
    int incr = c.incr;
    for (int i=0; i<c.numtuples; i++) {
        cout << *((int*)addr) << endl;
        addr += incr;
    }
    return 0;
}
