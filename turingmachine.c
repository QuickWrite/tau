#include "turingmachine.h"

void next_state(struct TuringMachine* const machine) {
    const struct Rule rule = machine->state->rules[machine->tape.content[machine->tape.cursor]];
    
    write(&machine->tape, rule.write_symbol);

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
