#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <string>
#include <vector>

constexpr const char *COLOR_RESET   = "\x1b[0m";
constexpr const char *COLOR_GREEN   = "\x1b[32m";
constexpr const char *COLOR_YELLOW  = "\x1b[33m";
constexpr const char *COLOR_RED     = "\x1b[31m";
constexpr const char *COLOR_CYAN    = "\x1b[36m";
constexpr const char *COLOR_MAGENTA = "\x1b[35m";
constexpr const char *COLOR_BLUE    = "\x1b[34m";


int readChoice();
// Pre: None
// Post: Prints the input prompt "> " and reads an integer from standard input; returns it, or 0
//       if the input was not a number. Either way it then discards the REST OF THE LINE, so one
//       call consumes exactly one line -- which is what lets it be interleaved with readLine
//       without leaving a stray newline for the next reader to trip over. The cost is that
//       several numbers on one line no longer queue up as several answers.
//       Throws std::runtime_error if the input stream is closed (EOF).

std::string readLine();
// Pre: None
// Post: Reads a line from standard input and returns it without the newline; an empty line comes
//       back as an empty string, which callers are expected to read as "no answer" and fill in a
//       default for. Throws std::runtime_error if the input stream is closed (EOF).

std::string pickRandom(const std::vector<std::string> &v, std::mt19937 &rng,
                       const std::string &avoid = "");
// Pre: v must not be empty (throws std::runtime_error otherwise).
// Post: Returns a uniformly random element of v, re-drawing while it equals `avoid` -- so a
//       caller that feeds back the line it printed last never gets the same one twice in a row.
//       A one-element list is returned as it is, avoid or not: the guard against an endless
//       re-draw matters more than the repetition. Callers that pass nothing are unaffected.

void enableConsoleColors();
// Pre: None
// Post: Enables ANSI escape sequence processing (colors) in the Windows console;
//       silently does nothing if the console does not support it (e.g. redirected output).

#endif
