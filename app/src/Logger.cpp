#include "Logger.h"
#include <iostream>

TeeBuf::TeeBuf(std::streambuf *console, std::streambuf *file) 
    : console_(console), file_(file) {}

int TeeBuf::overflow(int c) {
    if (c != traits_type::eof()) {
        console_->sputc(static_cast<char>(c));

        if (inEscape_) {
            if (c == 'm')
                inEscape_ = false;
        } else if (c == '\x1b')
            inEscape_ = true;
        else 
            file_->sputc(static_cast<char>(c));
    }
    return c;
}

int TeeBuf::sync() {
    int consoleResult = console_->pubsync();
    int fileResult = file_->pubsync();
    if (consoleResult == 0 && fileResult == 0)
        return 0;
    return -1;
}

SessionLog::SessionLog(const std::string &path)
    : file_(path), tee_(std::cout.rdbuf(), file_.rdbuf()), original_(nullptr) {
        if (file_)
            original_ = std::cout.rdbuf(&tee_);
        else
            std::cout << "Warning: could not open " << path
                        << ". The session will not be recorded." << std::endl;
    }

    SessionLog::~SessionLog() {
        if (original_ != nullptr)
            std::cout.rdbuf(original_);
    }