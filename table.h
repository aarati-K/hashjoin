#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <string.h>

using namespace std;

#ifndef _TABLE_H_
#define _TABLE_H_

enum ColumnType {
    CT_INT,     // integer
    CT_LONG,    // ulong
    CT_FLOAT,   // float
    CT_CHAR,    // single char
    CT_STRING   // string of chars, up to a max size, null terminated
};

struct ColumnInfo {
    ColumnType ct;
    void* startAddr = 0;
    ulong incr = 0;
    int numtuples = 0;
    ulong offset;
};

typedef struct ColumnInfo ColumnInfo;

class Table {
public:
    Table();
    void addCol(ColumnType ct, ulong size);
    int numCols();
    void printCols();
    void printColOffsets();
    int getNumTuples();
    ulong getTupleSize();
    void loadFromFile(string fname, char sep);
    ColumnInfo getColumnInfo(int col);
    void shuffle();
private:
    void* buf;
    vector<ColumnType> cols;
    vector<ulong> colsize;
    vector<ulong> coloffset;
    ulong tuplesize;
    int numtuples;
    bool initialized;
};

#endif // _TABLE_H_
