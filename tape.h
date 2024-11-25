#pragma once

#include <stdlib.h>

typedef short Symbol;

struct Tape {
    Symbol* content;
    size_t size;

    char** symbol_names;

    //

    Symbol def;

    size_t cursor;
};

struct Tape init_tape_full(const Symbol def, Symbol* content, const size_t size);

struct Tape init_tape(const Symbol def);

void left(struct Tape* const tape);

void right(struct Tape* const tape);

void write(struct Tape* const tape, const Symbol symbol);

void free_tape(struct Tape* const tape);
