//
// Created by xardb on 11/27/25.
//
#include "arg_parser.hpp"
#include <iostream>
#include <cctype>
#include <vector>

using namespace std;

// Эта функция мне нужна чтобы разбить введённую строку на аргументы

vector<string> split_args(const string& input) {
    vector<string> args;
    string arg;
    bool in_quotes = false; //всё внутри кавычек считаем одним аргументом

    for (char c : input) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (isspace(c) && !in_quotes) {
            if (!arg.empty()) {
                args.push_back(arg);
                arg.clear();
            }
        } else {
            arg += c;
        }
    }

    if (!arg.empty()) args.push_back(arg);
    return args;
}
