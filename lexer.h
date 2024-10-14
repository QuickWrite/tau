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

struct Lexer init_lexer(const char* const fileName);

struct Token get_next_token(struct Lexer* const lexer);
