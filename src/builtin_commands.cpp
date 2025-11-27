//
// Created by xardb on 11/27/25.
//

#include "builtin_commands.hpp"
#include <iostream>
#include <cstdlib>

using namespace std;

extern void check_mbr(const string&);
extern void read_gpt(const string&);

bool handle_builtin(const string& input) {

    if (input == "\\q") {
        exit(0);
    }

    if (input.rfind("echo ", 0) == 0 || input.rfind("debug ", 0) == 0) {
        size_t pos = input.find(' ');
        if (pos != string::npos) {
            string text = input.substr(pos + 1);
            if (text.size() >= 2 && text.front() == '\'' && text.back() == '\'')
                text = text.substr(1, text.size() - 2);
            cout << text << endl;
        }
        return true;
    }

    if (input.rfind("\\e ", 0) == 0) {
        string var = input.substr(3);
        if (var.empty() || var[0] != '$') {
            cout << "Неверная команда. Надо: \\e $VARNAME\n";
            return true;
        }

        char* val = getenv(var.substr(1).c_str());
        if (!val) return true;

        string env = val;
        size_t start = 0, pos;

        while ((pos = env.find(':', start)) != string::npos) {
            cout << env.substr(start, pos - start) << endl;
            start = pos + 1;
        }
        cout << env.substr(start) << endl;
        return true;
    }

    if (input.rfind("\\l ", 0) == 0) {
        string dev = input.substr(3);
        check_mbr(dev);
        read_gpt(dev);
        return true;
    }

    return false;
}
