 #include <iostream>
#include <string>
#include <fstream>

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
        if (input == "\\q") break;
        else if (input.find("echo ") == 0) {
            cout << input.substr(5) << endl;
        }
        //Добавляем  \e для вывода переменных окружения
        // else if(input.find("\\e") == 0){
        //     string varname = input.substr(3);
        //     char* value = getenv(varname.c_str());
        //     if
        // }



        else{
            cout << "Неизвестная команда: " << input << endl;
        }

    }
    write_file.close();
    cout << "Выход" << endl;
}