#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <string>
#include <vector>

int readChoice();
// Pre: None
// Post: Prints the input prompt "> " and reads an integer from standard input;
//       returns it, or 0 if the input was not a number (the bad line is discarded).

std::string pickRandom(const std::vector<std::string> &v, std::mt19937 &rng);
// Pre: v must not be empty (throws std::runtime_error otherwise).
// Post: Returns a uniformly random element of v.

#endif
