#include <iostream>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <map>
#include <vector>

const char uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char lowercase[] = "abcdefghijklmnopqrstuvwxyz";
const char number[] = "01234567890";
const char especial[] = "$&()*+[]@#^-_!?";

enum { EXIT, GENERATE_PASSWORD, IS_PWNED };

void print_banner(){
    std::cout << "                                                        " << std::endl;
    std::cout << "                                                        " << std::endl;
    std::cout << "    █████▄  ▄▄▄   ▄▄▄▄  ▄▄▄▄                            " << std::endl;
    std::cout << "    ██▄▄█▀ ██▀██ ███▄▄ ███▄▄                            " << std::endl;
    std::cout << "    ██     ██▀██ ▄▄██▀ ▄▄██▀                            " << std::endl;
    std::cout << "                                                        " << std::endl;
    std::cout << " ▄████  ▄▄▄▄▄ ▄▄  ▄▄ ▄▄▄▄▄ ▄▄▄▄   ▄▄▄ ▄▄▄▄▄▄ ▄▄▄  ▄▄▄▄  " << std::endl;
    std::cout << "██  ▄▄▄ ██▄▄  ███▄██ ██▄▄  ██▄█▄ ██▀██  ██  ██▀██ ██▄█▄ " << std::endl;
    std::cout << " ▀███▀  ██▄▄▄ ██ ▀██ ██▄▄▄ ██ ██ ██▀██  ██  ▀███▀ ██ ██ " << std::endl;
    std::cout << "       Autor: rev1insane (https://github.com/rev1nsane) " << std::endl;
    std::cout << std::endl;
}

void print_password(std::string password, bool pwned){
    const int col1 = password.length() + 2;
    const int col2 = 5;

    std::cout << "+" << std::setw(col1+2) << std::setfill('-') << "-"
              << "+" << std::setw(col2+2) << "-" << "+" << std::setfill(' ') << "\n";

    std::cout << "| " << std::setw(col1) << std::left << "PASSWORD"
              << " | " << std::setw(col2) << std::left << "PWNED" << " |\n";

    std::cout << "+" << std::setw(col1+2) << std::setfill('-') << "-"
              << "+" << std::setw(col2+2) << "-" << "+" << std::setfill(' ') << "\n";

    std::cout << "| " << std::setw(col1)  << password
              << " | " << std::setw(col2) << (pwned ? "true" : "false")
              << " |\n";

    std::cout << "+" << std::setw(col1+2) << std::setfill('-') << "-"
              << "+" << std::setw(col2+2) << "-" << "+" << std::setfill(' ') << "\n";
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string query_pwned_api(const std::string& prefix) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (!curl) {
        std::cerr << "Failed to init curl\n";
        return "";
    }

    std::string url = "https://api.pwnedpasswords.com/range/" + prefix;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "password-checker");

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl);
    return response;
}

std::string sha1_to_hex(const unsigned char hash[20]) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    for (int i = 0; i < 20; i++) {
        ss << std::setw(2) << (int)hash[i];
    }

    return ss.str();
}

std::string generate_pass(){
    int minLenPassword = 8, maxLenPassword = 20;
    std::string passwordGenerated;
    char character;
    int lenPasswordGenerated = 0, lenPassword = 0;
    int typeCharacter, position;

    do {
        std::cout << "Password size(8-20): "; std::cin >> lenPassword;

        if ( lenPassword >= minLenPassword && lenPassword <= maxLenPassword) {
            while( lenPasswordGenerated < lenPassword ){

                typeCharacter = 1 + rand() % 4;
                switch ( typeCharacter ) {
                    case 1: {
                        position = rand() % strlen(uppercase);
                        character = uppercase[position];
                    } break;
                    case 2: {
                        position =  rand() % strlen(lowercase);
                        character = lowercase[position];
                    } break;
                    case 3: {
                        position = rand() % strlen(number);
                        character = number[position];
                    } break;
                    case 4: {
                        position = rand() % strlen(especial);
                        character = especial[position];
                    } break;
                }
                passwordGenerated.push_back(character);
                lenPasswordGenerated++;
            }
        } else {
            std::cout << "The password must be between 8 and 20 characters long." << std::endl;
        }
    } while (lenPassword < minLenPassword || lenPassword > maxLenPassword);

    passwordGenerated.push_back('\0');
    return passwordGenerated;
}


std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
        end = s.find(delimiter, start);
    }

    tokens.push_back(s.substr(start));
    return tokens;
}

std::map<std::string, int> map_response(std::string api_response){
    std::map<std::string, int> pwned_hashes;

    std::vector<std::string> lines = split(api_response, "\n");
    for ( std::string line: lines ) {
        std::vector<std::string> one_hash_with_count = split(line, ":");
        std::string hash = one_hash_with_count[0];
        int count = std::stoi(one_hash_with_count[1]);
        pwned_hashes[hash] = count;
    }

    return pwned_hashes;
}

bool pass_is_pwned(std::string passwordGenerated){
    std::map<std::string, int> pwned_hashes;
    unsigned char hash[SHA_DIGEST_LENGTH]; // SHA_DIGEST_LENGTH == 20
    int pass_size = passwordGenerated.length();

    SHA1(reinterpret_cast<const unsigned char*>(passwordGenerated.c_str()), pass_size, hash);

    std::string hex = sha1_to_hex(hash);
    std::string prefix = hex.substr(0, 5);
    std::string api_response = query_pwned_api(prefix);
    pwned_hashes = map_response(api_response);

    for (const auto& [hash, count] : pwned_hashes) {
        if ( hex.substr(5) == hash ) {
            return true;
        }
    }
    return false;
}


int menu(){
    int opt = 0;
    int tries = 0;

    do {
        if ( opt != 1 && opt != 2 && tries >= 1 ) std::cout << "[x] Wrong option." << std::endl << std::endl;
        std::cout << " Menu " << std::endl;
        std::cout << "1. Generate strong password." << std::endl;
        std::cout << "2. Password have been pwned." << std::endl;
        std::cout << "0. Exit." << std::endl;
        std::cout << "Enter option: ";  std::cin >> opt;
        tries++;
    } while ( opt != 1 && opt != 2 && opt != 0);

    return opt;
}


int main()
{
    srand(time(0));
    print_banner();
    bool pass_pwned = false;
    int opt = 0;
    opt = menu();

    if ( opt == EXIT ) return 0;

    if ( opt == GENERATE_PASSWORD ) {
        std::string passwordGenerated = generate_pass();
        pass_pwned = pass_is_pwned(passwordGenerated);
        print_password(passwordGenerated, pass_pwned);
    }
    if ( opt == IS_PWNED ) {
        std::string input_password;
        std::cout << "[+] Enter password: "; std::cin >> input_password;
        pass_pwned = pass_is_pwned(input_password);
        print_password(input_password, pass_pwned);
    }


    return 0;
}
