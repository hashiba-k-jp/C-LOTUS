#include "util.hpp"

/* Logger */
void Logger::info(const string& msg){
    cout << "\033[32m[INFO ] " << msg << "\033[39m\n";
};

void Logger::warn(const string& msg){
    cout << "\033[33m[WARN ] " << msg << "\033[39m\n";
};

void Logger::error(const string& msg){
    cout << "\033[31m[ERROR] " << msg << "\033[39m\n";
};
