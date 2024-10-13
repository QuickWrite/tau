#include "lexer.h"

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

struct Lexer initLexer(const char* const fileName) {
    FILE* fptr = fopen(fileName, "r");

    struct Lexer lexer = {
        .fptr = fptr
    };

    return lexer;
}

void skipWhitespace(FILE* const fptr) {
    int c;
    while((c = getc(fptr)) != EOF && isspace(c));

    ungetc(c, fptr);
}

void skipComment(FILE* const fptr) {
    int c;
    while((c = getc(fptr)) != EOF && c != '\n');
}

void skipDelimiter(FILE* const fptr) {
    int c;
    while((c = getc(fptr)) != EOF && c == '-');

    ungetc(c, fptr);
}

char* getIdentifier(FILE* const fptr, int c) {
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

char* getNumber(FILE* const fptr, int c) {
    char* buf = calloc(16, sizeof(char));
    buf[0] = c;

    int i = 1;
    for(; i < 16; ++i) {
        c = getc(fptr);

        if(!isnumber(c)) {
            break;
        }

        buf[i] = c;
    }

    assert(i < 16 && "Numbers cannot be larger than '16' characters long.");
    
    ungetc(c, fptr);

    return buf;
}

struct Token getNextToken(struct Lexer* const lexer) {
    skipWhitespace(lexer->fptr);

    int c = getc(lexer->fptr);
    switch (c) {
    case EOF:
        return (struct Token){.type = END_OF_FILE};

    case '#':
        // This is a comment. It should not be returned as a token as it is completely useless for a parser
        skipComment(lexer->fptr);

        return getNextToken(lexer);
        
    case '=':
        return (struct Token){.type = EQUALS};
    case ',':
        return (struct Token){.type = COMMA};
    case '{':
        return (struct Token){.type = CURLY_OPEN};
    case '}':
        return (struct Token){.type = CURLY_CLOSE};

    case '-':
        skipDelimiter(lexer->fptr);

        return (struct Token){.type = DELIMITER};

    default:
        if(isalpha(c)) {
            return (struct Token){
                .type = IDENTIFIER, 
                .content = getIdentifier(lexer->fptr, c)
            };
        }

        if(isnumber(c)) {
            return (struct Token){
                .type = NUMBER, 
                .content = getNumber(lexer->fptr, c)
            };
        }

        assert(0 && "This is still TODO");

        // Something went wrong
        break;
    }

    return (struct Token){0};
}

