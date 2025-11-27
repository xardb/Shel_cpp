#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <pthread.h>
#include <pwd.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "arg_parser.hpp"
#include "command_path.hpp"
#include "builtin_commands.hpp"
#include "history.hpp"
#include "executor.hpp"
#include "vfs_manager.hpp"
#include "vfs.hpp"

using namespace std;
//обработчик сигнала
void handle_sighup(int){
    cout << "\nConfiguration reloaded" << endl;
}
int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;


    fuse_start();
    signal(SIGHUP, handle_sighup);
    auto history = open_history();

    string input;
    while (true) {
        cerr << "₽ " << flush;
        if (!getline(cin, input)) break;
        if (input.empty()) continue;

        append_history(history, input);

        if (handle_builtin(input))
            continue;

        vector<string> args = split_args(input);
        if (args.empty()) continue;

        string cmd = find_command_path(args[0]);
        if (cmd.empty()) {
            cout << args[0] << ": command not found\n";
            continue;
        }

        execute_external(cmd, args);
    }

    return 0;
}
