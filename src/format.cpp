#include <string>
#include <memory>
#include <iostream>

#include "format.h"

#define TIME_STR_SIZE 9

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    auto buffer = std::make_unique<char[]>(TIME_STR_SIZE);

    int hh = seconds / (3600);
    seconds = seconds % 3600;
    int mm = seconds / 60;
    int ss = seconds % 60;

    std::snprintf(buffer.get(), TIME_STR_SIZE, "%02d:%02d:%02d", hh, mm, ss);

    return std::string(buffer.get(), buffer.get() + TIME_STR_SIZE);
}