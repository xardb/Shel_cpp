 #include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main() {
    system("chcp 65001");
    string input;
    cout << "мой шелл :)" << endl;
    while (true){
        cout << "₽ ";
        if (!getline(cin, input)) {
            break; // Ctrl+D
        }
        if (input.empty()) continue;
        if (input == "\\q") break;

    }

}