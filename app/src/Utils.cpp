#include "Utils.h"

#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <limits>
#include <stdexcept>



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
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode))
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
