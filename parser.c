#include "parser.h"

#include <stdio.h>
#include <string.h>

#define TO_END goto end

struct Head {
    Symbol blank;
    Symbol* symbols;
    size_t symbol_len;

    Symbol* tape;
    size_t tape_len;

    char* start_state;
    char* end_state;
};

static size_t parse_symbol_list(struct Lexer* const lexer, Symbol** const symbols) {
    size_t allocated = 16;
    *symbols = malloc(sizeof(Symbol) * allocated);

    size_t size = 0;

    while (1) {
        next_token(lexer);
        
        if (lexer->curr_token.type == END_OF_FILE) {
            // TODO: Way better error

            printf("End of file whilst parsing list.\n");
            exit(10);
        }

        if (lexer->curr_token.type != NUMBER) {
            // TODO: Way better error

            printf("Lists currently only support numbers.\n");
            exit(10);
        }

        if(allocated < size + 1) {
            realloc(*symbols, sizeof(Symbol) * allocated * 2);
            allocated *= 2;
        }

        (*symbols)[size] = atoi(lexer->curr_token.content);
        ++size;

        free(lexer->curr_token.content);

        if (lexer->next_token.type != COMMA) {
            break;
        }
        
        // Remove token comma
        next_token(lexer);
    };
    
    realloc(*symbols, sizeof(Symbol) * size);

    return size;
}

static void parse_statement(struct Lexer* const lexer, struct Head* head, const char* const name) {
    next_token(lexer);

    if (lexer->curr_token.type != EQUALS) {
        // TODO: Way better error

        printf("Statement has to have a `=`.\n");
        exit(10);
    }
    
    if(strcmp(name, "blank") == 0) {
        // TODO: Add identifier support

        next_token(lexer);

        if (lexer->curr_token.type != NUMBER) {
            printf("Content of 'blank' has to be a number.\n");
            exit(10);
        }
        
        head->blank = atoi(lexer->curr_token.content);

        free(lexer->curr_token.content);

        TO_END;
    }

    if (strcmp(name, "start") == 0) {
        next_token(lexer);

        if (lexer->curr_token.type != IDENTIFIER) {
            printf("Content of 'start' has to be an identifier.\n");
            exit(10);
        }

        head->start_state = lexer->curr_token.content;

        TO_END;
    }
    
    if (strcmp(name, "end") == 0) {
        next_token(lexer);

        if (lexer->curr_token.type != IDENTIFIER) {
            printf("Content of 'end' has to be an identifier.\n");
            exit(10);
        }

        head->end_state = lexer->curr_token.content;

        TO_END;
    }

    if (strcmp(name, "symbols") == 0) {
        head->symbol_len = parse_symbol_list(lexer, &head->symbols);

        TO_END;
    }

    if (strcmp(name, "tape") == 0) {
        head->tape_len = parse_symbol_list(lexer, &head->tape);

        TO_END;
    }

end:
    free(name);
}

static void parse_head(struct Lexer* const lexer, struct Head* head) {
    next_token(lexer);

    for (;lexer->curr_token.type != DELIMITER; next_token(lexer)) {
        if (lexer->curr_token.type == END_OF_FILE) {
            // TODO: Way better error

            printf("End of file reached before end of head.\n");
            exit(10);
        }

        if(lexer->curr_token.type != IDENTIFIER) {
            // TODO: Way better error

            printf("Statement has to start with an identifier.\n");
            exit(10);
        }

        parse_statement(lexer, head, lexer->curr_token.content);
    }

    if (head->start_state == NULL) {
        printf("A starting state has to be defined.\n");
        exit(10);
    }

    if (head->symbol_len == 0) {
        printf("A tape needs to have symbols.\n");
        exit(10);
    }
}

struct TuringMachine* parse(struct Lexer* const lexer) {
    struct Head head = {0};
    parse_head(lexer, &head);

    return NULL;
}

