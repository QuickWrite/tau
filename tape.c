#include "tape.h"

#include <stdio.h>
#include <string.h>

static void set_default(Symbol* const start, const size_t length, const Symbol def) {
    for(size_t i = 0; i < length; i++){
        ((Symbol*)start)[i] = def;
    }
}

struct Tape init_tape_full(const Symbol def, Symbol* content, const size_t size) {
    struct Tape const tape = {
        .content = content,
        .size = size,
        .def = def,
        .cursor = size / 2
    };

    return tape;
}

struct Tape init_tape(const Symbol def) {
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
        fprintf(stderr, "Not enough memory\n");

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
        fprintf(stderr, "Not enough memory\n");

        exit(EXIT_FAILURE);
    }

    tape->content = new;
    set_default(tape->content + tape->size, tape->size, tape->def);

    tape->size *= 2;
}

void write(struct Tape* const tape, const Symbol symbol) {
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

    printf("\nsize: %zu\n", tape->size);
    printf("default: %i\n", tape->def);
    printf("cursor: %zu\n", tape->cursor);    
}