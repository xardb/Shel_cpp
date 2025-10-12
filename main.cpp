 #include <iostream>
#include <string>
#include <fstream>
#include <cstdlib> 
#include <sstream>

using namespace std;

int main() {
    // system("chcp 65001");
    string input;
    string history_file = "kubsh_history.txt";
    ofstream write_file(history_file, ios::app);
    cout << "мой шелл :)" << endl;
    while (true){
        cout << "₽ ";
        if (!getline(cin, input)) {
            break; // Ctrl+D
        }
        if (input.empty()) continue;
        write_file << input << endl;
        write_file.flush();


        if (input == "\\q") {
            break;
        }
        else if (input.find("echo ") == 0) {
            cout << input.substr(5) << endl;
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
                    // Разбиваем по ':' и выводим каждую часть с новой строки
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
            } else {
                cout << "Переменная окружения '" << var_name << "' не найдена" << endl;
            }
        }
        else{
            cout << "Неизвестная команда: " << input << endl;
        }

    }
    write_file.close();
    cout << "Выход" << endl;
}