//
// Created by xardb on 11/27/25.
//

#include "command_path.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;
// Функция для поиска полного пути к команде
    string find_command_path(const string& command) {
        if (command.find('/') != string::npos) {
            if (access(command.c_str(), X_OK) == 0) {
                return command;
            }
            return "";
        }
        //если в нашей команде есть / - значит нам дали прямой путь
        // acess() проверяет наши права на выполнение
        char* path_env = getenv("PATH");
        if (!path_env) return "";

        stringstream paths(path_env);
        string path;

        while (getline(paths, path, ':')) {
            string full_path = path + "/" + command;
            if (access(full_path.c_str(), X_OK) == 0) {
                return full_path;
            }
        }
        //Разбиваем путь по : на отдельные директории
        // для каждой проверяем, есть ли там наш бинарник
        // и если нашли - возвращаем исходный путь
        return "";
    }
