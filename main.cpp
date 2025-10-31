 #include <iostream>
#include <string>
#include <fstream>
#include <cstdlib> 
#include <sstream>
#include <vector> // чтоб хранить аргументы
#include <unistd.h> //fork, exec,wait
#include <sys/wait.h> // waitpid для ожидание смерти дочерних процессов
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
    
    if (!arg.empty()) {
        args.push_back(arg);
    }
    
    return args;
}

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

int main() {
    // system("chcp 65001");
    string input;
    string history_file = "kubsh_history.txt";
    ofstream write_file(history_file, ios::app);
    while (true){
        // cout << "₽ ";
        if (!getline(cin, input)) {
            break; // Ctrl+D
        }
        if (input.empty()) continue;
        write_file << input << endl;
        write_file.flush();


        if (input == "\\q") {
            break;
        }


        else if ((input.find("echo ") == 0) || input.find("debug ") == 0){
            size_t pos = input.find(' ');
            if(pos != string::npos){
                string text = input.substr(pos+1);

                if (text.size() >= 2 && text.front() == '\'' && text.back() == '\'') {
                    text = text.substr(1, text.size() - 2);
                }
                cout << text << endl;
            }
        }


        else if (input.find("\\e ") == 0) {
            // Команда \e для вывода переменных окружения
            string var_name = input.substr(3);
            //ПРоверка на доляр
            if(var_name.empty() || var_name[0] !='$'){
                cout << "Неверная команда. Надо: \\e $VARNAME" << endl;
                continue;
            }

            string var_namewd = var_name.substr(1);
            char* value = getenv(var_namewd.c_str());
            
            if (value != nullptr) {
                string env_value = value;
                // Проверяем, есть ли ':'
               if (env_value.find(':') != string::npos) {
                    // Разбиваем по ':' и выводим каждую часть 
                    size_t start = 0;
                    size_t end = env_value.find(':');
                    while (end != string::npos) {
                        cout << env_value.substr(start, end - start) << endl;
                        start = end + 1;
                        end = env_value.find(':', start);
                    }
                    // Выводим последний элемент
                    cout << env_value.substr(start) << endl;
                } else {
                    cout << env_value << endl;
                }
            } 
            
        }
        else{
            // Если не q, не \e и не эхо, то считаем команду внешней
            vector<string> args = split_args(input); //и разбиваем её на аргументы
            if(args.empty()) continue;

            //Ищем полный путь к команде ls = /bin/ls
            string command_path = find_command_path(args[0]);

            //не нашли команду - вывели ошибку и продолжили цикл
            if(command_path.empty()){
                cout << args[0] << ": command not found" << endl;
                continue;
            }

            vector<char*> c_args; // хранятся указатели на аргументы в формате чар, требование execvp
            vector<string> string_storage; //копия строк чтоб они не уничтожались до выполнения команды
            for(auto arg:args){
                string_storage.push_back(arg);
                c_args.push_back(&string_storage.back()[0]); //указатель на первый символ
            }
            c_args.push_back(nullptr); //execvp хочет нульптр в конце
            
            pid_t pid = fork(); // Создали копию текущего процесса // 0 - мы в дочернем, > 0 в родительском, <0 ошибка 
            if (pid == 0){
                //Дочерний процесс
                execvp(command_path.c_str(), c_args.data()); //заменяем нашу программу на целевую команду
                // Если вернули управление - ошибка
                cerr << "Ошибка выполнения: " << args[0] << endl;
                exit(1); 
            }
            else if(pid >0 ){
                int status;
                waitpid(pid, &status, 0); // в родительском процессе ждем завершение дочернего, waitpid блокирует выполнение пока ребенок не помрет
            }else{
                cerr << "Ошибка при создании процесса";
            }
        }
        
    }
    write_file.close();
}