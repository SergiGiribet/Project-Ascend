#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <string>
#include <vector>

constexpr const char *COLOR_RESET  = "\x1b[0m";
constexpr const char *COLOR_GREEN  = "\x1b[32m";
constexpr const char *COLOR_YELLOW = "\x1b[33m";
constexpr const char *COLOR_RED    = "\x1b[31m";
constexpr const char *COLOR_CYAN   = "\x1b[36m";


int readChoice();
// Pre: None
// Post: Prints the input prompt "> " and reads an integer from standard input;
//       returns it, or 0 if the input was not a number (the bad line is discarded).
//       Throws std::runtime_error if the input stream is closed (EOF).

std::string pickRandom(const std::vector<std::string> &v, std::mt19937 &rng);
// Pre: v must not be empty (throws std::runtime_error otherwise).
// Post: Returns a uniformly random element of v.

void enableConsoleColors();
// Pre: None
// Post: Enables ANSI escape sequence processing (colors) in the Windows console;
//       silently does nothing if the console does not support it (e.g. redirected output).

#endif
