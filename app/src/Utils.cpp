#include "Utils.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>
#include <limits>
#include <stdexcept>
#include <chrono>
#include <cstdlib>
#include <algorithm>


int readChoice()
{
    std::cout << "> ";
    int c;
    if (!(std::cin >> c))
    {
        if (std::cin.eof())
            throw std::runtime_error("Input stream is closed.");

        std::cin.clear();
        c = 0;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return c;
}

std::string readLine()
{
    std::string line;
    if (!std::getline(std::cin, line))
    {
        if (std::cin.eof())
            throw std::runtime_error("Input stream is closed.");
        else
            throw std::runtime_error("Error reading input.");
    }
    return line;
}

std::string pickRandom(const std::vector<std::string> &v, std::mt19937 &rng,
                       const std::string &avoid) // the default lives in the header, not here
{
    if (v.empty())
        throw std::runtime_error("pickRandom: the list is empty.");
    std::uniform_int_distribution<size_t> dist(0, v.size() - 1);
    std::string picked = v[dist(rng)];
    if (v.size() > 1)
        while (picked == avoid)
            picked = v[dist(rng)];
    return picked;
}

void enableConsoleColors() {
    #ifdef _WIN32
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode))
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif
}

long long nowSeconds()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::seconds since = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    return since.count();
}

std::tm localTime(std::time_t t)
{
    std::tm tm{};
    #ifdef _WIN32
    localtime_s(&tm, &t);
    #else
    localtime_r(&t, &tm);
    #endif
    return tm;
}

// MSVC deprecates getenv because the pointer it returns can be invalidated by a later putenv in
// the same process. Nothing here ever sets an environment variable, and the value is copied into
// an int before the call returns, so the pointer never outlives the statement that reads it.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
static int readScale()
{
    const char *raw = std::getenv("ASCEND_TIME_SCALE");
    if (raw == nullptr)
        return 1;
    return std::max(1, std::atoi(raw));
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

int timeScale()
{
    static const int scale = readScale();
    return scale;
}

long long realSeconds(int gameMinutes)
{
    return static_cast<long long>(gameMinutes) * 60 / timeScale();
}
