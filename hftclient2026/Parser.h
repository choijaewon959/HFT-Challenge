//
// Created by Jae Won Choi on 15/5/2026.
//

#ifndef HIGH_FREQUENCY_TRADING_CHALLENGE_PARSER_H
#define HIGH_FREQUENCY_TRADING_CHALLENGE_PARSER_H

#include <iostream>
#include <netinet/in.h>
#include <cstring>

class Parser {
private:
    int sock;
    std::string buf;
    static const int BUFFER_SIZE = 4096;

public:
    Parser(int s) : sock(s) {}

    bool readInt(int& x) {
        while (true) {
            // Remove leading whitespace
            size_t i = 0;
            while (i < buf.size() && isspace((unsigned char)buf[i])) {
                i++;
            }
            if (i > 0) {
                buf.erase(0, i);
            }

            // Find end of integer token
            size_t j = 0;
            while (j < buf.size() && !isspace((unsigned char)buf[j])) {
                j++;
            }

            // We have a complete token only if it ends with whitespace
            if (j < buf.size()) {
                std::string token = buf.substr(0, j);
                buf.erase(0, j);

                if (!token.empty()) {
                    x = stoi(token);
                    return true;
                }
            }

            // Need more bytes
            char temp[BUFFER_SIZE];
            int bytes = recv(sock, temp, BUFFER_SIZE, 0);

            if (bytes <= 0) {
                return false;
            }

            buf.append(temp, bytes);
        }
    }
};

class FastParser {
private:
    int sock;
    static const int BUFFER_SIZE = 1 << 17; // 128KB
    char buff[BUFFER_SIZE];
    int pos = 0;
    int len = 0;

    bool refill() {
        // Calculate how many bytes are left unconsummed in the buffer
        int leftover = len - pos;
        if (leftover > 0) {
            // Move the partial number/token to the very front of the buffer
            std::memmove(buff, buff + pos, leftover);
            pos = 0;
            len = leftover;
        } else {
            pos = 0;
            len = 0;
        }

        // Read from socket into the remaining free space of the buffer
        int bytes_received = recv(sock, buff + len, BUFFER_SIZE - len, 0);
        if (bytes_received <= 0) return false; // Error or disconnection

        len += bytes_received;
        return true;
    }

    bool getChar(char& c) {
        if (pos >= len) {
            if (!refill()) return false;
        }
        c = buff[pos++];
        return true;
    }


public:
    FastParser(int s) : sock(s) {}

    bool readInt(int& x) {
        char c;
        // skip whitespace-like character
        do {
            if (!getChar(c)) return false;
        } while (c <= ' ');

        // assign new int to x
        x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            if (!getChar(c)) return true;
        }

        return true;
    }
};

#endif //HIGH_FREQUENCY_TRADING_CHALLENGE_PARSER_H