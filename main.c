#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef short elem;

typedef struct {
    elem* content;
    size_t size;

    //

    elem def;

    size_t cursor;
} Tape;

static void set_default(elem* const start, const size_t length, const elem def) {
    for(size_t i = 0; i < length; i++){
        ((elem*)start)[i] = def;
    }
}

Tape* const init_tape(const elem def, const size_t size) {
    Tape* const tape = malloc(sizeof *tape);
    
    tape->content = malloc(sizeof(elem) * size);

    set_default(tape->content, size, def);

    tape->size = size;

    tape->def = def;
    tape->cursor = size / 2;

    return tape;
}

void left(Tape* const tape) {
    if(tape->cursor > 0) {
        --tape->cursor;
        return;
    }

    elem* new = malloc(sizeof(elem) * tape->size * 2);

    if(!new) {
        printf("Not enough memory\n");

        exit(EXIT_FAILURE);
    }

    memcpy(new + tape->size, tape->content, sizeof(elem) * tape->size);
    set_default(new, tape->size, tape->def);

    tape->content = new;

    tape->cursor += tape->size - 1;
    tape->size *= 2;
}

void right(Tape* const tape) {
    if(!(tape->size < ++(tape->cursor))) {
        return;
    }

    elem* new = realloc(tape->content, sizeof(elem) * tape->size * 2);

    if(!new) {
        printf("Not enough memory\n");

        exit(EXIT_FAILURE);
    }

    tape->content = new;
    set_default(tape->content + tape->size, tape->size, tape->def);

    tape->size *= 2;
}

void free_tape(Tape* const tape) {
    free(tape->content);

    free(tape);
}

void print(const Tape* const tape){
    printf("Contents of tape: \n");
    printf("Content: ");
    for (size_t i = 0; i < tape->size; i++) {
        printf("%i,", tape->content[i]);
    }

    printf("\nsize: %i\n", tape->size);
    printf("default: %i\n", tape->def);
    printf("cursor: %i\n", tape->cursor);    
}

/**
 * Example of the tape
 */
int main(const int argc, const char** const argv) {
    Tape* const tape = init_tape(0, 5);

    for (size_t i = 1; i < 11; i++){
        left(tape);

        tape->content[tape->cursor] = i;

        print(tape);
        printf("----------------------\n");
    }
    
    free_tape(tape);
}
