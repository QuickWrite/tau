#pragma once

#include "tape.h"

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

void next_state(struct TuringMachine* const machine);
