#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cctype>
#include <windows.h>

using namespace std;

// Базовый класс для всех шифров
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

    string getAlphabet() const { return cipherAlphabet; }
};

// Шифр Виженера
class VigenereCipher : public Cipher {
    // храним ключ в верхнем регистре, только латинские буквы
    string keyUpper;
public:
    VigenereCipher(const string& key) {
        for (char c : key) keyUpper += static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }

    string encrypt(const string& text) override {
        string result;
        int keyLen = static_cast<int>(keyUpper.size());
        int ki = 0; // индекс по ключу — увеличиваем только при обработке букв
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
                char mapped = static_cast<char>('A' + ( (t - k + 26) % 26 ));
                result += (upper ? mapped : static_cast<char>(tolower(mapped)));
                ++ki;
            } else {
                result += c;
            }
        }
        return result;
    }

    string getKeyUpper() const { return keyUpper; }
};

// Помощник для безопасного чтения целого числа из строки
static bool read_int(const string& prompt, int& out) {
    string line;
    cout << prompt;
    if (!getline(cin, line)) return false;
    try {
        size_t pos;
        int val = stoi(line, &pos);
        if (pos != line.size()) return false;
        out = val;
        return true;
    } catch (...) {
        return false;
    }
}

// Строгая проверка ключевого слова: только латинские буквы, непустой
static bool read_keyword_strict(const string& prompt, string& outKey) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) return false;
    if (line.empty()) return false;
    for (char c : line) {
        if (!isalpha(static_cast<unsigned char>(c))) return false;
        char up = static_cast<char>(toupper(static_cast<unsigned char>(c)));
        if (up < 'A' || up > 'Z') return false;
    }
    outKey = line;
    return true;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    vector<string> algorithms = { "Шифр Цезаря", "Одноалфавитный шифр", "Шифр Виженера" };

    while (true) {
        cout << "Меню\n";
        cout << "1) Шифровать\n";
        cout << "2) Расшифровать\n";
        cout << "0) Выход\n";

        int mode;
        if (!read_int("Выберите режим: ", mode)) {
            cout << "Неверный ввод. Введите число.\n";
            continue;
        }
        if (mode == 0) break;
        if (mode != 1 && mode != 2) {
            cout << "Неверный режим. Попробуйте снова.\n";
            continue;
        }

        cout << "\nДоступные шифры:\n";
        for (size_t i = 0; i < algorithms.size(); ++i) {
            cout << (i + 1) << ") " << algorithms[i] << "\n";
        }

        int choice;
        if (!read_int("Выберите шифр (номер): ", choice)) {
            cout << "Неверный ввод. Введите число.\n";
            continue;
        }
        if (choice < 1 || static_cast<size_t>(choice) > algorithms.size()) {
            cout << "Неверный номер шифра.\n";
            continue;
        }

        unique_ptr<Cipher> cipher;
        string alphabetForShow;
        string vigenereKeyShow;

        if (algorithms[choice - 1] == "Шифр Цезаря") {
            int key;
            while (true) {
                if (!read_int("Введите ключ (целое неотрицательное число): ", key)) {
                    cout << "Неверный ввод ключа. Попробуйте снова.\n";
                    continue;
                }
                if (key < 0) {
                    cout << "Ключ должен быть неотрицательным. Попробуйте снова.\n";
                    continue;
                }
                break;
            }
            cipher = make_unique<CaesarCipher>(key);

        } else if (algorithms[choice - 1] == "Одноалфавитный шифр") {
            string key;
            while (true) {
                if (!read_keyword_strict("Введите ключевое слово (только латинские буквы, без пробелов и знаков): ", key)) {
                    cout << "Ключевое слово должно содержать только латинские буквы и быть непустым. Попробуйте снова.\n";
                    continue;
                }
                break;
            }
            auto kc = make_unique<MonoCipher>(key);
            alphabetForShow = kc->getAlphabet();
            cipher = move(kc);

        } else if (algorithms[choice - 1] == "Шифр Виженера") {
            string key;
            while (true) {
                if (!read_keyword_strict("Введите ключ для Виженера (только латинские буквы, без пробелов): ", key)) {
                    cout << "Ключ должен содержать только латинские латинские буквы и быть непустым. Попробуйте снова.\n";
                    continue;
                }
                break;
            }
            auto vc = make_unique<VigenereCipher>(key);
            vigenereKeyShow = vc->getKeyUpper();
            cipher = move(vc);
        } else {
            cout << "Выбран неизвестный шифр.\n";
            continue;
        }

        string text;
        cout << "Введите текст: ";
        if (!getline(cin, text)) {
            cout << "Ошибка ввода.\n";
            break;
        }

        if (!alphabetForShow.empty()) {
            cout << "Алфавит шифра (Keyword): " << alphabetForShow << endl;
        }
        if (!vigenereKeyShow.empty()) {
            cout << "Ключ Виженера (верхний регистр): " << vigenereKeyShow << endl;
        }

        string result;
        try {
            if (mode == 1) result = cipher->encrypt(text);
            else result = cipher->decrypt(text);
        } catch (const exception& e) {
            cout << "Ошибка при обработке: " << e.what() << "\n";
            continue;
        }

        cout << "Результат: " << result << endl;
    }

    return 0;
}