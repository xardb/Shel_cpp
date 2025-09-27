 #include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main() {
    system("chcp 65001");
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

    }
    write_file.close();
    cout << "Выход" << endl;
}