#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#define new_token(name) (struct Token){.type = TOK_EQUALS}

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
    size_t size = 16;
    char* buf = calloc(size, sizeof(char));
    if(buf ==  NULL) {
        fprintf(stderr, "Couldn't allocate enough memory.\n");
        exit(100);
    }

    buf[0] = c;

    size_t i = 1;
    for(; i < 255; ++i) {
        c = getc(fptr);

        if(!isalnum(c) && c != '_') {
            break;
        }

        if(i >= size) {
            size *= 2;
            buf = realloc(buf, sizeof(char) * size);

            if(buf ==  NULL) {
                fprintf(stderr, "Couldn't allocate enough memory.\n");
                exit(100);
            }
        }

        buf[i] = c;
    }

    if(i > 255) {
        fprintf(stderr, "Identifiers cannot be larger than '255' characters long.");
    }

    buf[i] = '\0';
    buf = realloc(buf, sizeof(char) * (i + 1));
    
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
        next = (struct Token){.type = TOK_EOF};
        break;

    case '#':
        // This is a comment. It should not be returned as a token as it is completely useless for a parser
        skip_comment(lexer->fptr);

        next_token(lexer);
        return;
        
    case '=':
        next = new_token(TOK_EQUALS);
        break;
    case ',':
        next = new_token(TOK_COMMA);
        break;
    case '_':
        next = new_token(TOK_UNDERSCORE);
        break;
    case '{':
        next = new_token(TOK_OPEN_CURLY);
        break;
    case '}':
        next = new_token(TOKEN_CLOSE_CURLY);
        break;

    case '-':
        skip_delimiter(lexer->fptr);

        next = new_token(TOKEN_DELIMITER);
        break;

    default:
        if(isalpha(c)) {
            next = new_token(TOK_IDENTIFIER);
            next.content = get_identifier(lexer->fptr, c);

            break;
        }

        if(isdigit(c)) {
            next = new_token(TOK_NUMBER);
            next.content = get_number(lexer->fptr, c);
            
            break;
        }

        assert(0 && "This is still TODO");

        // Something went wrong
        break;
    }

    lexer->curr_token = lexer->next_token;
    lexer->next_token = next;
}

