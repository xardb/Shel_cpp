//
// Created by xardb on 11/27/25.
//

#include "builtin_commands.hpp"
#include <iostream>
#include <cstdlib>

using namespace std;

extern void check_mbr(const string&);
extern void read_gpt(const string&);
//принимает строку, смотрит является ли команда встроенной, если да - выполняет
bool handle_builtin(const string& input) {

    if (input == "\\q") {
        exit(0); //Пункт 3
    }

    //Пункт 5
    if (input.rfind("echo ", 0) == 0 || input.rfind("debug ", 0) == 0) {
        size_t pos = input.find(' ');// нашли первый пробел(граница между командой и текстом)
            if (pos != string::npos) {
            string text = input.substr(pos + 1); // берем весь текст после пробела
            if (text.size() >= 2 && text.front() == '\'' && text.back() == '\'')
                text = text.substr(1, text.size() - 2); // находим и убираем кавычки
            cout << text << endl;
        }
        return true;
    }

    //7. Добавим команду по выводу переменной окружения (`\e $PATH`, списком, если есть `:`)
    if (input.rfind("\\e ", 0) == 0) {
        string var = input.substr(3); //отрезаем "\e "

        if (var.empty() || var[0] != '$') {
            cout << "Неверная команда. Надо: \\e $VARNAME\n";
            return true;
        }

        char* val = getenv(var.substr(1).c_str()); // получаем переменную окружения через getenv
        if (!val) return true;

        string env = val;
        size_t start = 0, pos;

        //Парсим значения по : как PATH. чтоб разбить вывод
        while ((pos = env.find(':', start)) != string::npos) {
            cout << env.substr(start, pos - start) << endl;
            start = pos + 1;
        }
        cout << env.substr(start) << endl;
        return true;
    }

    //задание 9 - диски
    if (input.rfind("\\l ", 0) == 0) {
        string dev = input.substr(3);
        check_mbr(dev); //печатает таблицу mbr разделов
        read_gpt(dev); //печатает таблицу gpt разделов
        return true;
    }

    return false; // если дошли до сюда - отправляем команду во внешнее выполнение..
}
