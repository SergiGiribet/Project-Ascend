#include "Utils.h"

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
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        c = 0;
    }
    return c;
}

std::string pickRandom(const std::vector<std::string> &v, std::mt19937 &rng)
{
    if (v.empty())
        throw std::runtime_error("pickRandom: the list is empty.");
    std::uniform_int_distribution<size_t> dist(0, v.size() - 1);
    return v[dist(rng)];
}
