#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "error.h"
#include "tape.h"

#define TO_END goto end

#define CHECK_TOKEN(token_name, message) if (lexer->curr_token.type != token_name) { \
            fseek(lexer->fptr, lexer->curr_token.position, SEEK_SET); \
            print_parser_error(lexer->fptr, lexer->file_name, "Invalid Token", message); \
            exit(10); \
        }

struct Head {
    char* blank;
    Symbol blank_number;
    bool blank_defined;

    char** symbols;
    size_t symbol_len;

    char** tape_elems;
    Symbol* tape;
    size_t tape_len;

    char* start_state;
    char* end_state;
};

static char* halt = "HALT";

inline static size_t find_symbol(const struct Head* const head, const char* const name) {
    for(size_t i = 0; i < head->symbol_len; ++i) {
        if(strcmp(head->symbols[i], name) == 0) {
            return i;
        }
    }

    return (size_t)-1;
}

static size_t parse_symbol_list(struct Lexer* const lexer, char*** const symbols) {
    size_t allocated = 16;
    *symbols = malloc(sizeof(Symbol) * allocated);

    size_t size = 0;

    while (1) {
        next_token(lexer);
        
        if (lexer->curr_token.type == TOK_EOF) {
            // TODO: Way better error

            fprintf(stderr, "End of file whilst parsing list.\n");
            exit(10);
        }

        CHECK_TOKEN(TOK_IDENTIFIER, "A list can only contain identifiers.");

        if(allocated < size + 1) {
            *symbols = realloc(*symbols, sizeof(Symbol) * allocated * 2);

            if (*symbols == NULL) {
                fprintf(stderr, "Error whilst reallocating array.\n");
                exit(1);
            }
            
            allocated *= 2;
        }

        (*symbols)[size] = lexer->curr_token.content;
        ++size;

        if (lexer->next_token.type != TOK_COMMA) {
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

#define CHECK_MULTIPLE_DEFINITION(check, message) if (check) { \
            print_parser_error(lexer->fptr, lexer->file_name, "Multiple Definition", message); \
            exit(10); \
        }

static void parse_statement(struct Lexer* const lexer, struct Head* head, const char* const name) {
    next_token(lexer);

    CHECK_TOKEN(TOK_EQUALS, "Statement has to have a `=`.");
    
    if(strcmp(name, "blank") == 0) {
        CHECK_MULTIPLE_DEFINITION(head->blank_defined, "Blank specified multiple times.");

        next_token(lexer);

        CHECK_TOKEN(TOK_IDENTIFIER, "Content of 'blank' has to be an identifier.");
        
        head->blank = lexer->curr_token.content;
        head->blank_defined = true;

        TO_END;
    }

    if (strcmp(name, "start") == 0) {
        CHECK_MULTIPLE_DEFINITION(head->start_state != 0, "Start state specified multiple times.");

        next_token(lexer);

        CHECK_TOKEN(TOK_IDENTIFIER, "Content of 'start' has to be an identifier.");

        head->start_state = lexer->curr_token.content;

        TO_END;
    }
    
    if (strcmp(name, "end") == 0) {
        CHECK_MULTIPLE_DEFINITION(head->end_state != 0, "End state specified multiple times.");

        next_token(lexer);

        CHECK_TOKEN(TOK_IDENTIFIER, "Content of 'end' has to be an identifier.");

        head->end_state = lexer->curr_token.content;

        TO_END;
    }

    if (strcmp(name, "symbols") == 0) {
        CHECK_MULTIPLE_DEFINITION(head->symbol_len != 0, "Symbols were specified multiple times.");

        head->symbol_len = parse_symbol_list(lexer, &head->symbols);

        TO_END;
    }

    if (strcmp(name, "tape") == 0) {
        CHECK_MULTIPLE_DEFINITION(head->tape_len != 0, "Tape was specified multiple times.");

        head->tape_len = parse_symbol_list(lexer, &head->tape_elems);

        TO_END;
    }

end:
    free((char*)name);
}

#undef CHECK_MULTIPLE_DEFINITION

static void parse_head(struct Lexer* const lexer, struct Head* head) {
    *head = (struct Head){0};

    next_token(lexer);

    for (;lexer->curr_token.type != TOK_DELIMITER; next_token(lexer)) {
        if (lexer->curr_token.type == TOK_EOF) {
            // TODO: Way better error

            fprintf(stderr, "End of file reached before end of head.\n");
            exit(10);
        }

        if(lexer->curr_token.type != TOK_IDENTIFIER) {
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

    if(head->end_state == NULL) {
        head->end_state = halt;

        fprintf(stdout, "End state not defined. Assuming 'HALT' as end state.\n");
    }

    if(head->blank_defined) {
        head->blank_number = find_symbol(head, head->blank);
    }

    // If the tape is larger than 0, the tape needs to be corrected.
    if(head->tape_len > 0) {
        head->tape = malloc(sizeof(Symbol) * head->tape_len);

        for(size_t i = 0; i < head->tape_len; ++i) {
            head->tape[i] = (Symbol)find_symbol(head, head->tape_elems[i]);
        }
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

void parse_rule(struct Lexer* const lexer, const struct Head* head, struct IntermediateRule* rule) {
    CHECK_TOKEN(TOK_IDENTIFIER, "A symbol is an identifier.");

    rule->rule.write_symbol = find_symbol(head, lexer->curr_token.content);
    if(rule->rule.write_symbol == -1) {
        fprintf(stderr, "Did not find rule with name '%s'.", lexer->curr_token.content);
        exit(10);
    }
    free(lexer->curr_token.content);

    next_token(lexer);

    CHECK_TOKEN(TOK_COMMA, "A rule has to be separated with commas.");
    
    next_token(lexer);
    enum Direction dir;

    CHECK_TOKEN(TOK_IDENTIFIER, "A direction has to be RIGHT/R, LEFT/L, STAY/S");

    if((dir = stdirection(lexer->curr_token.content)) == (enum Direction)-1) {
        fprintf(stderr, "The direction of a rule has to be RIGHT/R, LEFT/L, STAY/S. \"%s\" is not allowed.\n", lexer->curr_token.content);
        exit(10);
    }

    free(lexer->curr_token.content);

    rule->rule.direction = dir;

    next_token(lexer);

    CHECK_TOKEN(TOK_COMMA, "A rule has to be separated with commas.");

    next_token(lexer);
    CHECK_TOKEN(TOK_IDENTIFIER, "The next state has to be an identifier.");

    rule->next_state = lexer->curr_token.content;
}

void parse_state(struct Lexer* const lexer, const struct Head* head, struct IntermediateState* state) {
    CHECK_TOKEN(TOK_IDENTIFIER, "State declaration has to begin with an identifier.");

    state->name = lexer->curr_token.content;

    next_token(lexer);
    CHECK_TOKEN(TOK_OPEN_CURLY, "State declaration has to open with '{'.");
    
    state->rules = calloc(head->symbol_len, sizeof(struct IntermediateRule));
    if (state->rules == NULL) {
        fprintf(stderr, "Not enough memory could be allocated.\n");
        exit(1);
    }

    while (lexer->next_token.type != TOK_CLOSE_CURLY && lexer->next_token.type != TOK_EOF) {
        next_token(lexer);
        if(lexer->curr_token.type != TOK_IDENTIFIER && lexer->curr_token.type != TOK_UNDERSCORE) {
            // TODO: Add to new error system
            fprintf(stderr, "Only identifiers are supported as Symbols. The default can be declared with `_`.\n");
            exit(10);
        }

        size_t position = lexer->curr_token.type == TOK_UNDERSCORE ? (size_t)-1 : find_symbol(head, lexer->curr_token.content);

        if(lexer->curr_token.type == TOK_IDENTIFIER) {
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
        CHECK_TOKEN(TOK_EQUALS, "Declaration of statement has to be in the form of <symbol> = <new symbol>, <mov>, <next>.");

        next_token(lexer);
        parse_rule(lexer, head, position == (size_t)-1 ? &state->def : &state->rules[position]);
    }
    next_token(lexer);

    CHECK_TOKEN(TOK_CLOSE_CURLY, "State declaration has to end with a }, but found EOF.");
}

size_t parse_body(struct Lexer* const lexer, struct IntermediateState* states[], const struct Head* const head) {
    size_t size = 16;
    
    *states = calloc(size, sizeof(struct IntermediateState));
    size_t amount = 0;

    while(lexer->curr_token.type != TOK_EOF) {
        if(amount >= size) {
            *states = realloc(*states, size * 2);

            if(*states == NULL) {
                fprintf(stderr, "Error whilst reallocating array.\n");
                exit(1);
            }

            size = size * 2;
        }

        parse_state(lexer, head, &(*states)[amount]);
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

        free(int_state->rules[i].next_state);
    }

    if(int_state->def.next_state == NULL) {
        if(missing_rule) {
            fprintf(stderr, "The state '%s' is not exhaustive. Maybe create an unspecified rule.\n", state->name);
            exit(10);
        }

        return;
    }

    // Caching the next state as it is quite an expensive task.
    struct State* def_next_state = NULL;

    // If the next state is the end state, then it does not have
    // to try to find the correct state and can just keep it as
    // NULL.
    if(strcmp(int_state->def.next_state, end_state) != 0) {
        for(size_t j = 0; j < states_size; ++j) {
            if(strcmp(int_states[j].name, int_state->def.next_state) != 0) {
                continue;
            }
            def_next_state = states + j;
            break;
        }
        if(def_next_state == NULL) {
            fprintf(stderr, "The rule '%s' was used but never defined.\n", int_state->def.next_state);
            exit(10);
        }
    }

    for(size_t i = 0; i < rules_size; ++i) {
        if(state->rules[i].next_state != NULL) {
            continue;
        }

        state->rules[i].direction = int_state->def.rule.direction;
        state->rules[i].next_state = def_next_state;
        state->rules[i].write_symbol = int_state->def.rule.write_symbol;
    }
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

    struct State* states = link_states(int_states, states_size, head.symbol_len, head.end_state);

    // Free all intermediate states
    for(size_t i = 0; i < states_size; ++i) {
        free(int_states[i].rules);
    }
    free(int_states);

    struct State* start = find_start(states, states_size, head.start_state);
    if(start == NULL) {
        fprintf(stderr, "Referencing starting state does not exist: '%s'.\n", head.start_state);
    }

    struct TuringMachine* machine = calloc(1, sizeof(struct TuringMachine));
    machine->state = start;

    if(head.tape_len > 0) {
        Symbol* symbols = malloc(sizeof(Symbol) * head.tape_len);

        for(size_t i = 0; i < head.tape_len; ++i) {
            symbols[i] = head.tape[i];
        }

        machine->tape = init_tape_full(head.blank_number, symbols, head.tape_len);
    } else {
        machine->tape = init_tape(head.blank_number);
    }

    machine->tape.symbol_names = head.symbols;

    // Free the head
    free(head.blank);
    free(head.tape_elems);

    if(head.end_state != halt) {
        free(head.end_state);
    }
    
    free(head.start_state);

    return machine;
}

