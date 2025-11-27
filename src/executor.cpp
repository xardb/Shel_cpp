//
// Created by xardb on 11/27/25.
//
#include "executor.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
using namespace std;

void execute_external(const string& path, const vector<string>& args) {
    vector<char*> cargs;
    vector<string> storage;

    for (auto& s : args) {
        storage.push_back(s);
        cargs.push_back(&storage.back()[0]);
    }
    cargs.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        execvp(path.c_str(), cargs.data());
        cerr << "Ошибка выполнения: " << args[0] << endl;
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        cerr << "Ошибка при создании процесса\n";
    }
}
