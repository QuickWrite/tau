#include <stdio.h>
#include <string.h>

#include "turingmachine.h"
#include "parser.h"

void print_machine(const struct TuringMachine* const machine, const size_t view_width){
    size_t whitespace_len = 0;

    const struct Tape* const tape = &machine->tape;

    long top = view_width % 2 == 1 ? tape->cursor + (view_width / 2) + 1 : tape->cursor + (view_width / 2);


    printf("..., ");
    setvbuf(stdout, NULL, _IOLBF, 0);

    for(long i = tape->cursor - (view_width / 2); i < top; ++i) {
        printf(
                i == (long)tape->cursor ? "\033[31m%s\033[0m, " : "%s, ", 
                tape->symbol_names[i < 0 || i >= (long)tape->size ? tape->def : tape->content[i]]
            );

        if(i <= (long)tape->cursor) {
            whitespace_len += strlen(tape->symbol_names[i < 0 ? tape->def : tape->content[i]]) + 2;
        }
    }
    printf("...\n");
    char whitespace[whitespace_len + 1];
    memset(whitespace, ' ', whitespace_len);
    whitespace[whitespace_len] = '\0';
    printf("%s  ^\n", whitespace);

    if(machine->state != NULL) {
        printf("%s  \033[32m%s\033[0m\n", whitespace, tape->symbol_names[machine->state->rules[tape->content[tape->cursor]].write_symbol]);
    }
}

int main(const int argc, const char** const argv) {
    if(argc < 2) {
        fprintf(stderr, "A file path has to be provided: `tau <path>`\n");
        return 1;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);

    struct TuringMachine* machine = parse(argv[1]);

    printf("\033[1;4mExecution sequence:\033[0m\n\n");

    while(machine->state != NULL) {
        printf("State: %s\n", machine->state->name);
        print_machine(machine, 9);
        next_state(machine);
        printf("\n");
    }
    printf("State: HALT\n");
    print_machine(machine, 9);
}
