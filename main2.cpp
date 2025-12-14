#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <windows.h>
using namespace std;

class Cipher {
public:
    virtual string encrypt(const string& text) = 0;
    virtual string decrypt(const string& text) = 0;
    virtual ~Cipher() {}
};

class CaesarCipher : public Cipher {
    int shift;
public:
    CaesarCipher(int s) : shift(s) {}

    static bool is_alpha(char c) { return isalpha(static_cast<unsigned char>(c)); }
    static bool is_upper(char c) { return isupper(static_cast<unsigned char>(c)); }

    string encrypt(const string& text) override {
        string result = text;
        for (char& ch : result) {
            if (is_alpha(ch)) {
                char base = is_upper(ch) ? 'A' : 'a';
                ch = static_cast<char>(((ch - base + shift) % 26) + base);
            }
        }
        return result;
    }

    string decrypt(const string& text) override {
        string result = text;
        for (char& ch : result) {
            if (is_alpha(ch)) {
                char base = is_upper(ch) ? 'A' : 'a';
                ch = static_cast<char>(((ch - base - shift) % 26 + 26) % 26 + base);
            }
        }
        return result;
    }
};

class MonoCipher : public Cipher {
    string cipherAlphabet;
public:
    MonoCipher(const string& key) {
        cipherAlphabet = generateCipherAlphabet(key);
    }

    static string generateCipherAlphabet(const string& key) {
        string result;
        bool used[26] = { false };

        for (char c : key) {
            if (!isalpha(static_cast<unsigned char>(c))) continue;
            char up = static_cast<char>(toupper(static_cast<unsigned char>(c)));
            int idx = up - 'A';
            if (!used[idx]) {
                result += up;
                used[idx] = true;
            }
        }

        for (char c = 'A'; c <= 'Z'; ++c) {
            if (!used[c - 'A']) result += c;
        }

        return result;
    }

    string encrypt(const string& text) override {
        string result;
        for (char c : text) {
            if (isalpha(static_cast<unsigned char>(c))) {
                bool upper = isupper(static_cast<unsigned char>(c));
                char up = static_cast<char>(toupper(static_cast<unsigned char>(c)));
                char mapped = cipherAlphabet[up - 'A'];
                result += (upper ? mapped : static_cast<char>(tolower(mapped)));
            } else {
                result += c;
            }
        }
        return result;
    }

    string decrypt(const string& text) override {
        string result;
        for (char c : text) {
            if (isalpha(static_cast<unsigned char>(c))) {
                bool upper = isupper(static_cast<unsigned char>(c));
                char up = static_cast<char>(toupper(static_cast<unsigned char>(c)));
                size_t pos = cipherAlphabet.find(up);
                char orig = static_cast<char>('A' + static_cast<int>(pos));
                result += (upper ? orig : static_cast<char>(tolower(orig)));
            } else {
                result += c;
            }
        }
        return result;
    }
};

class VigenereCipher : public Cipher {
    string keyUpper;
public:
    VigenereCipher(const string& key) {
        for (char c : key) keyUpper += static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }

    string encrypt(const string& text) override {
        string result;
        int keyLen = static_cast<int>(keyUpper.size());
        int ki = 0;
        for (char c : text) {
            if (isalpha(static_cast<unsigned char>(c))) {
                bool upper = isupper(static_cast<unsigned char>(c));
                char up = static_cast<char>(toupper(static_cast<unsigned char>(c)));
                int t = up - 'A';
                int k = keyUpper[ki % keyLen] - 'A';
                char mapped = static_cast<char>('A' + (t + k) % 26);
                result += (upper ? mapped : static_cast<char>(tolower(mapped)));
                ++ki;
            } else {
                result += c;
            }
        }
        return result;
    }

    string decrypt(const string& text) override {
        string result;
        int keyLen = static_cast<int>(keyUpper.size());
        int ki = 0;
        for (char c : text) {
            if (isalpha(static_cast<unsigned char>(c))) {
                bool upper = isupper(static_cast<unsigned char>(c));
                char up = static_cast<char>(toupper(static_cast<unsigned char>(c)));
                int t = up - 'A';
                int k = keyUpper[ki % keyLen] - 'A';
                char mapped = static_cast<char>('A' + ((t - k + 26) % 26));
                result += (upper ? mapped : static_cast<char>(tolower(mapped)));
                ++ki;
            } else {
                result += c;
            }
        }
        return result;
    }
};
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const vector<string> names = { "Шифр Цезаря", "Одноалфавитный шифр", "Шифр Виженера" };

    while (true) {
        cout << "\nДоступные шифры:\n";
        for (size_t i = 0; i < names.size(); ++i) cout << (i + 1) << ") " << names[i] << '\n';
        cout << "0) Выход\n";

        int choice;
        cin >> choice;
        if (choice == 0) break;

        unique_ptr<Cipher> cipher;

        if (choice == 1) {
            int key;

            cin >> key;
            cipher = make_unique<CaesarCipher>(key);
        } else if (choice == 2) {
            string key;

            cin >> key;
            cipher = make_unique<MonoCipher>(key);
        } else if (choice == 3) {
            string key;

            cin >> key;
            cipher = make_unique<VigenereCipher>(key);
        } else {
            continue;
        }

        while (true) {
            cout << "\n1) Шифровать\n2) Расшифровать\n3) Другой шифр\n0) Выход\n";
            int act;
            cin >> act;
            if (act == 0) return 0;
            if (act == 3) break;

            cout << "Текст: ";
            string text;
            getline(cin >> ws, text); //здесь ws: удаляет ведущие пробелы/переводы строки
            string out = (act == 1) ? cipher->encrypt(text) : cipher->decrypt(text);
            cout << "Результат: " << out << endl;
        }
    }
    return 0;
}
