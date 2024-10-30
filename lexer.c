#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

struct Lexer init_lexer(const char* const fileName) {
    FILE* fptr = fopen(fileName, "r");

    if(fptr == NULL) {
        return (struct Lexer){0};
    }

    struct Lexer lexer = {
        .fptr = fptr
    };

    next_token(&lexer);

    return lexer;
}

void skip_whitespace(FILE* const fptr) {
    int c;
    while((c = getc(fptr)) != EOF && isspace(c));

    ungetc(c, fptr);
}

void skip_comment(FILE* const fptr) {
    int c;
    while((c = getc(fptr)) != EOF && c != '\n');
}

void skip_delimiter(FILE* const fptr) {
    int c;
    while((c = getc(fptr)) != EOF && c == '-');

    ungetc(c, fptr);
}

char* get_identifier(FILE* const fptr, int c) {
    char* buf = calloc(17, sizeof(char));
    buf[0] = c;

    int i = 1;
    for(; i < 16; ++i) {
        c = getc(fptr);

        if(!isalnum(c)) {
            break;
        }

        buf[i] = c;
    }

    if(i > 16) {
        fprintf(stderr, "Identifiers cannot be larger than '16' characters long.");
    }
    
    ungetc(c, fptr);

    return buf;
}

char* get_number(FILE* const fptr, int c) {
    char* buf = calloc(17, sizeof(char));
    buf[0] = c;

    int i = 1;
    for(; i < 16; ++i) {
        c = getc(fptr);

        if(!isdigit(c)) {
            break;
        }

        buf[i] = c;
    }

    if(i > 16) {
        fprintf(stderr, "Numbers cannot be larger than '16' characters long.");
    }
    
    ungetc(c, fptr);

    return buf;
}

void next_token(struct Lexer* const lexer) {
    skip_whitespace(lexer->fptr);

    struct Token next;

    int c = getc(lexer->fptr);
    switch (c) {
    case EOF:
        next = (struct Token){.type = END_OF_FILE};
        break;

    case '#':
        // This is a comment. It should not be returned as a token as it is completely useless for a parser
        skip_comment(lexer->fptr);

        next_token(lexer);
        return;
        
    case '=':
        next = (struct Token){.type = EQUALS};
        break;
    case ',':
        next = (struct Token){.type = COMMA};
        break;
    case '{':
        next = (struct Token){.type = CURLY_OPEN};
        break;
    case '}':
        next = (struct Token){.type = CURLY_CLOSE};
        break;

    case '-':
        skip_delimiter(lexer->fptr);

        next = (struct Token){.type = DELIMITER};
        break;

    default:
        if(isalpha(c)) {
            next = (struct Token){
                .type = IDENTIFIER, 
                .content = get_identifier(lexer->fptr, c)
            };
            break;
        }

        if(isdigit(c)) {
            next = (struct Token){
                .type = NUMBER, 
                .content = get_number(lexer->fptr, c)
            };
            break;
        }

        assert(0 && "This is still TODO");

        // Something went wrong
        break;
    }

    lexer->curr_token = lexer->next_token;
    lexer->next_token = next;
}

