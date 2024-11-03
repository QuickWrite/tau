#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"

#define TO_END goto end

struct Head {
    Symbol blank;
    bool blank_defined;

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

            fprintf(stderr, "End of file whilst parsing list.\n");
            exit(10);
        }

        if (lexer->curr_token.type != NUMBER) {
            // TODO: Way better error

            fprintf(stderr, "Lists currently only support numbers.\n");
            exit(10);
        }

        if(allocated < size + 1) {
            *symbols = realloc(*symbols, sizeof(Symbol) * allocated * 2);

            if (*symbols == NULL) {
                fprintf(stderr, "Error whilst reallocating array.\n");
                exit(1);
            }
            
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
    
    // Even though it should never change the pointer as this
    // will always shrink the amount that was allocated, this
    // uses the returned pointer in case there is some weird
    // stuff happening (Same reason why it is also checked).
    *symbols = realloc(*symbols, sizeof(Symbol) * size);
    if (*symbols == NULL) {
        fprintf(stderr, "Error whilst reallocating array.\n");
        exit(1);
    }

    return size;
}

static void parse_statement(struct Lexer* const lexer, struct Head* head, const char* const name) {
    next_token(lexer);

    if (lexer->curr_token.type != EQUALS) {
        // TODO: Way better error

        fprintf(stderr, "Statement has to have a `=`.\n");
        exit(10);
    }
    
    if(strcmp(name, "blank") == 0) {
        if (head->blank_defined) {
            fprintf(stderr, "Blank specified multiple times.\n");
            exit(10);
        }

        // TODO: Add identifier support

        next_token(lexer);

        if (lexer->curr_token.type != NUMBER) {
            fprintf(stderr, "Content of 'blank' has to be a number.\n");
            exit(10);
        }
        
        head->blank = atoi(lexer->curr_token.content);
        head->blank_defined = true;

        free(lexer->curr_token.content);

        TO_END;
    }

    if (strcmp(name, "start") == 0) {
        if (head->start_state != 0) {
            fprintf(stderr, "Start state specified multiple times.\n");
            exit(10);
        }

        next_token(lexer);

        if (lexer->curr_token.type != IDENTIFIER) {
            fprintf(stderr, "Content of 'start' has to be an identifier.\n");
            exit(10);
        }

        head->start_state = lexer->curr_token.content;

        TO_END;
    }
    
    if (strcmp(name, "end") == 0) {
        if (head->end_state != 0) {
            fprintf(stderr, "End state specified multiple times.\n");
            exit(10);
        }

        next_token(lexer);

        if (lexer->curr_token.type != IDENTIFIER) {
            fprintf(stderr, "Content of 'end' has to be an identifier.\n");
            exit(10);
        }

        head->end_state = lexer->curr_token.content;

        TO_END;
    }

    if (strcmp(name, "symbols") == 0) {
        if (head->symbol_len != 0) {
            fprintf(stderr, "Symbols were specified multiple times.\n");
            exit(10);
        }

        head->symbol_len = parse_symbol_list(lexer, &head->symbols);

        TO_END;
    }

    if (strcmp(name, "tape") == 0) {
        if (head->tape_len != 0) {
            fprintf(stderr, "Tape was specified multiple times.\n");
            exit(10);
        }

        head->tape_len = parse_symbol_list(lexer, &head->tape);

        TO_END;
    }

end:
    free((char*)name);
}

static void parse_head(struct Lexer* const lexer, struct Head* head) {
    *head = (struct Head){0};

    next_token(lexer);

    for (;lexer->curr_token.type != DELIMITER; next_token(lexer)) {
        if (lexer->curr_token.type == END_OF_FILE) {
            // TODO: Way better error

            fprintf(stderr, "End of file reached before end of head.\n");
            exit(10);
        }

        if(lexer->curr_token.type != IDENTIFIER) {
            // TODO: Way better error

            fprintf(stderr, "Statement has to start with an identifier.\n");
            exit(10);
        }

        parse_statement(lexer, head, lexer->curr_token.content);
    }

    if (head->start_state == NULL) {
        fprintf(stderr, "A starting state has to be defined.\n");
        exit(10);
    }

    if (head->symbol_len == 0) {
        fprintf(stderr, "A tape needs to have symbols.\n");
        exit(10);
    }
}

struct IntermediateRule {
    char* next_state;
    struct Rule rule;
};

struct IntermediateState {
    char* name;

    struct IntermediateRule def;
    struct IntermediateRule* rules;
};

enum Direction stdirection(const char* const string) {
    if(strcmp(string, "R") == 0 || strcmp(string, "RIGHT") == 0) {
        return RIGHT;
    }
    if (strcmp(string, "L") == 0 || strcmp(string, "LEFT") == 0) {
        return LEFT;
    }
    if (strcmp(string, "S") == 0 || strcmp(string, "STAY") == 0) {
        return STAY;
    }

    return -1;
}

size_t check_symbol(const Symbol* const symbols, const size_t symbol_len, const Symbol test) {
    for(size_t i = 0; i < symbol_len; ++i) {
        if(symbols[i] == test) {
            return i;
        }
    }

    // Even though a size_t does not have a -1, 
    // it is improbable that the highest number 
    // of elements is the max value of size_t.
    return -1;
}

void parse_rule(struct Lexer* const lexer, struct IntermediateRule* rule) {
    if(lexer->curr_token.type != NUMBER) {
        fprintf(stderr, "Only numbers are supported as Symbols.\n");
        exit(10);
    }
    rule->rule.write_symbol = atoi(lexer->curr_token.content);
    free(lexer->curr_token.content);

    next_token(lexer);
    if (lexer->curr_token.type != COMMA) {
        fprintf(stderr, "A rule has to be separated with commas.\n");
        exit(10);
    }
    
    next_token(lexer);
    enum Direction dir;
    if(lexer->curr_token.type != IDENTIFIER || (dir = stdirection(lexer->curr_token.content)) == (enum Direction)-1) {
        fprintf(stderr, "The direction of a rule has to be RIGHT/R, LEFT/L, STAY/S. \"%s\" is not allowed.\n", lexer->curr_token.content);
        exit(10);
    }

    rule->rule.direction = dir;

    next_token(lexer);
    if (lexer->curr_token.type != COMMA) {
        fprintf(stderr, "A rule has to be separated with commas.\n");
        exit(10);
    }

    next_token(lexer);
    if(lexer->curr_token.type != IDENTIFIER) {
        fprintf(stderr, "The next state has to be an identifier.\n");
        exit(10);
    }

    rule->next_state = lexer->curr_token.content;
}

void parse_state(struct Lexer* const lexer, struct IntermediateState* state, const Symbol* const symbols, const size_t symbol_len) {
    if(lexer->curr_token.type != IDENTIFIER) {
        fprintf(stderr, "State declaration has to begin with an identifier.\n");
        exit(10);
    }

    state->name = lexer->curr_token.content;

    next_token(lexer);
    if(lexer->curr_token.type != CURLY_OPEN) {
        fprintf(stderr, "State declaration has to open with '{'. %i\n", lexer->curr_token.type);
        exit(10);
    }
    
    state->rules = calloc(symbol_len, sizeof(struct IntermediateRule));
    if (state->rules == NULL) {
        fprintf(stderr, "Not enough memory could be allocated.\n");
        exit(1);
    }

    while (lexer->next_token.type != CURLY_CLOSE && lexer->next_token.type != END_OF_FILE) {
        next_token(lexer);
        if(lexer->curr_token.type != NUMBER && lexer->curr_token.type != UNDERSCORE) {
            fprintf(stderr, "Only numbers are supported as Symbols. The default can be declared with `_`.\n");
            exit(10);
        }

        size_t position = lexer->curr_token.type == UNDERSCORE ? -1 : check_symbol(symbols, symbol_len, atoi(lexer->curr_token.content));

        if(lexer->curr_token.type == NUMBER) {
            if (position == (size_t)-1) {
                fprintf(stderr, "Symbol %s does not exist in symbol list.\n", lexer->curr_token.content);
                exit(10);
            }

            if(state->rules[position].next_state != NULL) {
                fprintf(stderr, "The rule for '%s' cannot be declared twice.\n", lexer->curr_token.content);
                exit(10);
            }
            free(lexer->curr_token.content);
        } else {
            if(state->def.next_state != NULL) {
                fprintf(stderr, "The default rule cannot be declared twice.\n");
                exit(10);
            }
        }

        next_token(lexer);
        if(lexer->curr_token.type != EQUALS) {
            fprintf(stderr, "Declaration of statement has to be in the form of <symbol> = <new symbol>, <mov>, <next>\n");
            exit(10);
        }

        next_token(lexer);
        parse_rule(lexer, position == (size_t)-1 ? &state->def : &state->rules[position]);
    }
    next_token(lexer);
}

size_t parse_body(struct Lexer* const lexer, struct IntermediateState* states[], const struct Head* const head) {
    size_t size = 16;
    
    *states = malloc(sizeof(struct IntermediateState) * size);
    size_t amount = 0;

    while(lexer->curr_token.type != END_OF_FILE) {
        if(amount >= size) {
            *states = realloc(*states, size * 2);

            if(*states == NULL) {
                fprintf(stderr, "Error whilst reallocating array.\n");
                exit(1);
            }

            size = size * 2;
        }

        parse_state(lexer, &(*states)[amount], head->symbols, head->symbol_len);
        next_token(lexer);
        ++amount;
    }

    return amount;
}

static void link_rules(struct State* const state, const struct IntermediateState* const int_state, const size_t rules_size, 
                struct State* states, struct IntermediateState* const int_states, const size_t states_size, 
                const char* const end_state) {

    bool missing_rule = false;

    for(size_t i = 0; i < rules_size; ++i) {
        if(int_state->rules[i].next_state == NULL) {
            // If the rule does not exist, it will be zero initialized, so that it can be recognized later on.
            state->rules[i] = (struct Rule){0};
            printf("Missing rule set!\n");
            missing_rule = true;
            continue;
        }

        state->rules[i] = int_state->rules[i].rule;

        // If the next state is the end state, then it does not have
        // to try to find the correct state and can just keep it as
        // NULL.
        if(strcmp(int_state->rules[i].next_state, end_state) != 0) {
            for(size_t j = 0; j < states_size; ++j) {
                if(strcmp(int_states[j].name, int_state->rules[i].next_state) != 0) {
                    continue;
                }

                state->rules[i].next_state = states + j;
                break;
            }

            if(state->rules[i].next_state == NULL) {
                fprintf(stderr, "The rule '%s' was used but never defined.\n", int_state->rules[i].next_state);
                exit(10);
            }
        }
    }

    // TODO: Free those structs

    if(int_state->def.next_state == NULL) {
        if(missing_rule) {
            fprintf(stderr, "The state '%s' is not exhaustive. Maybe create an unspecified rule.\n", state->name);
            exit(10);
        }

        return;
    }

    // TODO: Add default handling
    printf("Currently the code should never reach this place. Wow.\n");
    exit(100);
}

struct State* link_states(struct IntermediateState* const int_states, const size_t size, const size_t rule_amount, const char* const end_state) {
    struct State* states = malloc(sizeof(struct State) * size);

    for(size_t i = 0; i < size; ++i) {
        states[i].name = int_states[i].name;

        states[i].rules = malloc(sizeof(struct Rule) * rule_amount);
        link_rules(&states[i], &int_states[i], rule_amount, states, int_states, size, end_state);
    }

    return states;
}

struct State* find_start(struct State* states, const size_t states_size, const char* const start) {
    for(size_t i = 0; i < states_size; ++i) {
        if(strcmp(states[i].name, start) == 0) {
            return &states[i];
        }
    }

    return NULL;
}

struct TuringMachine* parse(const char* const file_name) {
    struct Lexer lexer = init_lexer(file_name);
    if(lexer.fptr == NULL) {
        fprintf(stderr, "File path `%s` does not exist.\n", file_name);
        exit(1);
    }

    struct Head head;
    parse_head(&lexer, &head);

    // Remove that delimiter
    next_token(&lexer);

    struct IntermediateState* int_states;
    size_t states_size = parse_body(&lexer, &int_states, &head);

    fclose(lexer.fptr);

    // Some code that outputs some diagnostics
    // (It will be removed later on)
    // printf("Amount of states: %zu\n", states_size);
    // for(size_t i = 0; i < states_size; ++i) {
    //     printf("   %zu. \"%s\"\n", i, int_states[i].name);
    //     printf("   Default: Direction: '%i'\n", int_states[i].def.rule.direction);
    //     printf("   Rules:\n");
    //     for(size_t j = 0; j < head.symbol_len; ++j) {
    //         printf("      - Next: '%s', direction: '%i' \n", int_states[i].rules[j].next_state, int_states[i].rules[j].rule.direction);
    //     }
    // }

    struct State* states = link_states(int_states, states_size, head.symbol_len, head.end_state);

    struct State* start = find_start(states, states_size, head.start_state);
    if(start == NULL) {
        fprintf(stderr, "Referencing starting state does not exist: '%s'.\n", head.start_state);
    }

    struct TuringMachine* machine = calloc(1, sizeof(struct TuringMachine));
    machine->state = start;

    // TODO: Add default init tape
    machine->tape = *init_tape(head.blank);

    return machine;
}

