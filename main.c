#include <stdio.h>

#include "turingmachine.h"

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
