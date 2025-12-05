//
// Created by xardb on 11/27/25.
//
#include "arg_parser.hpp"
#include <iostream>
#include <cctype> // для isspace
#include <vector>

using namespace std;

// Эта функция мне нужна чтобы разбить введённую строку на аргументы
vector<string> split_args(const string& input) {
    vector<string> args; // вектор под готовые аргументы
    string arg; //буфер
    bool in_quotes = false; //всё внутри кавычек считаем одним аргументом

    for (char c : input) { // идем по каждому символу текущей строки
        if (c == '"') {
            in_quotes = !in_quotes; //обработка кавычек, чтоб они не попадали в итог
        } else if (isspace(c) && !in_quotes) { // если пробел и не в кавычка
            if (!arg.empty()) {     //значит пробел - разделитель аргументов
                args.push_back(arg);
                arg.clear();
            }
        } else {
            arg += c; // в любом другом случае добавляем символ к аргументу
        }
    }

    if (!arg.empty()) args.push_back(arg);
    return args;
}
