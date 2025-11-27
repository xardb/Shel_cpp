//
// Created by xardb on 11/27/25.
//

#include "history.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
using namespace std;

ofstream open_history() {
    string path = string(getenv("HOME") ? getenv("HOME") : ".") + "/.kubsh_history";
    return ofstream(path, ios::app);
}

void append_history(ofstream& f, const string& line) {
    f << line << endl;
    f.flush();
}
