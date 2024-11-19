#pragma once

#include <stdio.h>

enum TokenType {
    TOK_EOF = 0,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_EQUALS,
    TOK_COMMA,
    TOK_UNDERSCORE,
    TOK_OPEN_CURLY,
    TOK_CLOSE_CURLY,
    TOK_DELIMITER
};

struct Token {
    enum TokenType type;
    char* content;
    size_t position;
};

struct Lexer {
    FILE* fptr;
    const char* file_name;

    struct Token curr_token;
    struct Token next_token;   
};

struct Lexer init_lexer(const char* const fileName);

void next_token(struct Lexer* const lexer);
