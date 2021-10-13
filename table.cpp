#include "table.h"

Table::Table() {
    tuplesize = 0;
    numtuples = 0;
    buf = NULL;
    initialized = false;
}

void Table::addCol(ColumnType ct, ulong size) {
    cols.push_back(ct);
    coloffset.push_back(tuplesize);
    switch(ct) {
        case CT_INT:
            tuplesize += sizeof(int);
            colsize.push_back(sizeof(int));
            break;
        case CT_LONG:
            tuplesize += sizeof(ulong);
            colsize.push_back(sizeof(ulong));
            break;
        case CT_FLOAT:
            tuplesize += sizeof(float);
            colsize.push_back(sizeof(float));
            break;
        case CT_CHAR:
            tuplesize += sizeof(char);
            colsize.push_back(sizeof(char));
            break;
        case CT_STRING:
            tuplesize += size;
            colsize.push_back(size);
            break;
        default:
            tuplesize += 0;
    }
}

int Table::numCols() {
    return cols.size();
}

void Table::printCols() {
    cout << "Tuple Size: " << tuplesize << endl;
    vector<ColumnType>::iterator it;
    vector<ulong>::iterator szit;
    szit = colsize.begin();
    for (it = cols.begin(); it != cols.end(); it++, szit++) {
        switch(*it) {
            case CT_INT:
                cout << "Integer, " << *szit << endl;
                break;
            case CT_LONG:
                cout << "ulong, " << *szit << endl;
                break;
            case CT_FLOAT:
                cout << "Float, " << *szit << endl;
                break;
            case CT_CHAR:
                cout << "Char, " << *szit << endl;
                break;
            case CT_STRING:
                cout << "String, " << *szit << endl;
                break;
            default:
                break;
        }
    }
}

void Table::printColOffsets() {
    vector<ulong>::iterator it;
    for (it=coloffset.begin(); it!=coloffset.end(); it++) {
        cout << *it << endl;
    }
}

int Table::getNumTuples() {
    return numtuples;
}

ulong Table::getTupleSize() {
    return tuplesize;
}

void Table::loadFromFile(string fname, char sep) {
    ifstream infile;
    string line, tok;
    // vector<string> tokens;
    // vector<string>::iterator tok_it;
    vector<ColumnType>::iterator schema_it;
    vector<ulong>::iterator col_size_it;
    int numlines = 0;
    void* buf_it;

    // variables for parsing tokens
    int i;
    ulong l;
    float f;
    char c;

    // Count number of lines
    infile.open(fname);
    while (getline(infile, line)) {
        numlines += 1;
    }
    infile.close();

    // Allocate memory
    cout << "Allocating memory: " << numlines*tuplesize << " bytes" << endl;
    buf = (void*)malloc(numlines*tuplesize);
    if (buf == NULL) {
        cout << "Failed to initialize memory" << endl;
        return;
    }
    initialized = true;
    buf_it = buf;

    // Tokenize and store data
    infile.open(fname);
    while(getline(infile, line)) {
        // tokens.clear();
        // cout << line << endl;
        stringstream ss(line);
        schema_it = cols.begin();
        col_size_it = colsize.begin();
        while (getline(ss, tok, sep)) {
            if (schema_it == cols.end()) {
                cout << "More tokens than columns" << endl;
                cout << "Line: " << line << endl;
                return;
            }
            switch (*schema_it) {
                case CT_INT:
                    i = stoi(tok);
                    // cout << i << endl;
                    *((int*)buf_it) = i;
                    break;
                case CT_LONG:
                    l = stol(tok);
                    // cout << l << endl;
                    *((ulong*)buf_it) = l;
                    break;
                case CT_FLOAT:
                    f = stof(tok);
                    // cout << setprecision(2) << fixed << f << endl;
                    *((float*)buf_it) = f;
                    break;
                case CT_CHAR:
                    c = tok[0];
                    // cout << c << endl;
                    *((char*)buf_it) = c;
                    break;
                case CT_STRING:
                    strcpy((char*)buf_it, tok.c_str());
                    break;
                default:
                    break;
            }
            buf_it = (char*)buf_it + *col_size_it;
            schema_it++;
            col_size_it++;
        }
    }
    infile.close();
    numtuples = numlines;
}

ColumnInfo Table::getColumnInfo(int col) {
    ColumnInfo c;
    if (!initialized) {
        cout << "Table not initialized yet" << endl;
        return c;
    }
    if (col < 0 || col >= cols.size()) {
        cout << "Invalid input. col id ranges from 0 to " << cols.size() - 1 << endl;
        return c;
    }
    c.ct = cols.at(col);
    c.startAddr = (char*)buf + coloffset.at(col);
    c.offset = coloffset.at(col);
    c.incr = tuplesize;
    c.numtuples = numtuples;
    return c;
}

void __random_shuffle (int* array, ulong len) {
    ulong pos;
    uint swap;
    for (ulong i=len-1; i>0; i--) {
        pos = random()%(i+1);
        swap = array[i];
        array[i] = array[pos];
        array[pos] = swap;
    }
}

void Table::shuffle() {
    if (!initialized) {
        cout << "Table not initialized" << endl;
        return;
    }
    void* bufNew = malloc(numtuples*tuplesize);
    int *randOrder = (int*)malloc(sizeof(int)*numtuples);
    for (int i=0; i<numtuples; i++) {
        randOrder[i] = i;
    }
    for (int i=0; i<5; i++) {
        __random_shuffle(randOrder, numtuples);
    }

    for (int i=0; i<numtuples; i++) {
        memcpy((char*)bufNew + randOrder[i]*tuplesize, (char*)buf + i*tuplesize, tuplesize);
    }
    free(buf);
    buf = bufNew;
}
