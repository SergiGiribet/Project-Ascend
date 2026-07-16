#include "Utils.h"

int readChoice()
{
    int c;
    if (!(std::cin >> c))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        c = 0;
    }
    return c;
}