#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <streambuf>
#include <string>

class TeeBuf : public std::streambuf {
    public:
        TeeBuf(std::streambuf *console, std::streambuf *file);
        // Pre: console and file must outlive this TeeBuf.
        // Post: every character written through this buffer goes to both.

    protected:
        int overflow(int c) override;   // es crida per cada caracter: sputc als dos buffers
        int sync() override;            // flush: pubsync als dos

    private:
        std::streambuf *console_;
        std::streambuf *file_;
};

class SessionLog {
    public:
        explicit SessionLog(const std::string &path);
        // Post: opens path and mirrors all std::cout output to it; if the file
        //        cannot be opened, the game continues without logging. 

        ~SessionLog();
        // Post: restores std::cout's original buffer.

    private:
        std::ofstream file_;
        TeeBuf tee_;
        std::streambuf *original_;
};

#endif