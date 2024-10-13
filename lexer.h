#pragma once

#include <stdio.h>

enum TokenType {
    END_OF_FILE,
    IDENTIFIER,
    NUMBER,
    EQUALS,
    COMMA,
    CURLY_OPEN,
    CURLY_CLOSE,
    DELIMITER
};

struct Token {
    enum TokenType type;
    char* content;
};

struct Lexer {
    FILE* fptr;
};

struct Lexer initLexer(const char* const fileName);

struct Token getNextToken(struct Lexer* const lexer);
