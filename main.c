#include <stdio.h>

#include "turingmachine.h"
#include "parser.h"

int main(const int argc, const char** const argv) {
    if(argc < 2) {
        fprintf(stderr, "A file path has to be provided: `tau <path>`\n");
        return 1;
    }

    struct TuringMachine* machine = parse(argv[1]);

    while(machine->state != NULL) {
        printf("%s\n", machine->state->name);
        print(&machine->tape);
        next_state(machine);
        printf("----------------\n");
    }
    print(&machine->tape);
}
