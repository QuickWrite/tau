#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

struct Lexer init_lexer(const char* const fileName) {
    FILE* fptr = fopen(fileName, "r");

    struct Lexer lexer = {
        .fptr = fptr
    };

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
    char* buf = calloc(16, sizeof(char));
    buf[0] = c;

    int i = 1;
    for(; i < 16; ++i) {
        c = getc(fptr);

        if(!isalnum(c)) {
            break;
        }

        buf[i] = c;
    }

    assert(i < 16 && "Identifiers cannot be larger than '16' characters long.");
    
    ungetc(c, fptr);

    return buf;
}

char* get_number(FILE* const fptr, int c) {
    char* buf = calloc(16, sizeof(char));
    buf[0] = c;

    int i = 1;
    for(; i < 16; ++i) {
        c = getc(fptr);

        if(!isdigit(c)) {
            break;
        }

        buf[i] = c;
    }

    assert(i < 16 && "Numbers cannot be larger than '16' characters long.");
    
    ungetc(c, fptr);

    return buf;
}

struct Token get_next_token(struct Lexer* const lexer) {
    skip_whitespace(lexer->fptr);

    int c = getc(lexer->fptr);
    switch (c) {
    case EOF:
        return (struct Token){.type = END_OF_FILE};

    case '#':
        // This is a comment. It should not be returned as a token as it is completely useless for a parser
        skip_comment(lexer->fptr);

        return get_next_token(lexer);
        
    case '=':
        return (struct Token){.type = EQUALS};
    case ',':
        return (struct Token){.type = COMMA};
    case '{':
        return (struct Token){.type = CURLY_OPEN};
    case '}':
        return (struct Token){.type = CURLY_CLOSE};

    case '-':
        skip_delimiter(lexer->fptr);

        return (struct Token){.type = DELIMITER};

    default:
        if(isalpha(c)) {
            return (struct Token){
                .type = IDENTIFIER, 
                .content = get_identifier(lexer->fptr, c)
            };
        }

        if(isdigit(c)) {
            return (struct Token){
                .type = NUMBER, 
                .content = get_number(lexer->fptr, c)
            };
        }

        assert(0 && "This is still TODO");

        // Something went wrong
        break;
    }

    return (struct Token){0};
}

