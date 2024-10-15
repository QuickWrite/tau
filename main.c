#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef short Symbol;

struct Tape {
    Symbol* content;
    size_t size;

    //

    Symbol def;

    size_t cursor;
};

static void set_default(Symbol* const start, const size_t length, const Symbol def) {
    for(size_t i = 0; i < length; i++){
        ((Symbol*)start)[i] = def;
    }
}

struct Tape* init_tape_full(const Symbol def, Symbol* content, const size_t size) {
    struct Tape* const tape = malloc(sizeof *tape);
    
    tape->content = content;

    tape->size = size;

    tape->def = def;
    tape->cursor = size / 2;

    return tape;
}

struct Tape* init_tape(const Symbol def) {
    const size_t size = 16;
    Symbol* const content = malloc(sizeof(Symbol) * size);

    set_default(content, size, def);

    return init_tape_full(def, content, size);
}

void left(struct Tape* const tape) {
    if(tape->cursor > 0) {
        --tape->cursor;
        return;
    }

    Symbol* new = malloc(sizeof(Symbol) * tape->size * 2);

    if(!new) {
        printf("Not enough memory\n");

        exit(EXIT_FAILURE);
    }

    memcpy(new + tape->size, tape->content, sizeof(Symbol) * tape->size);
    set_default(new, tape->size, tape->def);

    free(tape->content);

    tape->content = new;

    tape->cursor += tape->size - 1;
    tape->size *= 2;
}

void right(struct Tape* const tape) {
    if(!(tape->size < ++(tape->cursor))) {
        return;
    }

    Symbol* new = realloc(tape->content, sizeof(Symbol) * tape->size * 2);

    if(!new) {
        printf("Not enough memory\n");

        exit(EXIT_FAILURE);
    }

    tape->content = new;
    set_default(tape->content + tape->size, tape->size, tape->def);

    tape->size *= 2;
}

void write_symbol(struct Tape* const tape, const Symbol symbol) {
    tape->content[tape->cursor] = symbol;
}

void free_tape(struct Tape* const tape) {
    free(tape->content);

    free(tape);
}

void print(const struct Tape* const tape){
    printf("Contents of tape: \n");
    printf("Content: ");
    for (size_t i = 0; i < tape->size; i++) {
        printf("%i,", tape->content[i]);
    }

    printf("\nsize: %i\n", tape->size);
    printf("default: %i\n", tape->def);
    printf("cursor: %i\n", tape->cursor);    
}

enum Direction {
    LEFT,
    RIGHT,
    STAY
};

struct Rule;

struct State {
    char* name;

    struct Rule* rules;
};

struct Rule {
    Symbol write_symbol;
    enum Direction direction;
    struct State* next_state;
};

struct TuringMachine {
    struct Tape tape;

    const struct State* state;
};

void next_state(struct TuringMachine* const machine) {
    const struct Rule rule = machine->state->rules[machine->tape.content[machine->tape.cursor]];
    
    write_symbol(&machine->tape, rule.write_symbol);

    machine->state = rule.next_state;

    switch (rule.direction)
    {
    case RIGHT:
        right(&machine->tape);
        break;
    case LEFT:
        left(&machine->tape);
        break;
    case STAY:
        // Do nothing
        break;
    }
}

/**
 * Example of the Turing Machine
 */
int main(const int argc, const char** const argv) {
    struct Tape* const tape = init_tape(0);
    struct TuringMachine machine = {
        .tape = *tape
    };

    struct State start = {
        .name = "START"
    };

    struct Rule rules[] = {
        {
            1,
            STAY,
            &start
        },
        {
            2,
            RIGHT,
            NULL
        }
    };
    
    start.rules = rules;

    machine.state = &start;

    while(machine.state != NULL) {
        printf("%s\n", machine.state->name);
        print(&machine.tape);
        next_state(&machine);
        printf("----------------\n");
    }
    print(&machine.tape);
}
