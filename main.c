#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef short elem;

typedef struct {
    elem* content;
    size_t size;

    //

    short def;

    size_t cursor;
} Tape;

void set_default(elem* start, size_t length, elem def) {
    for(size_t i = 0; i < length; i++){
        ((elem*)start)[i] = def;
    }
}

Tape* init_tape(elem def, size_t size) {
    Tape* tape = malloc(sizeof *tape);
    
    tape->content = malloc(sizeof(elem) * size);

    set_default(tape->content, size, def);

    tape->size = size;

    tape->def = def;
    tape->cursor = size / 2;

    return tape;
}

void left(Tape* tape) {
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

void right(Tape* tape) {
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

void free_tape(Tape* tape) {
    free(tape->content);

    free(tape);
}

void print(Tape* tape){
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
int main(int argc, char** argv) {
    Tape* tape = init_tape(0, 5);

    for (size_t i = 0; i < 10; i++){
        left(tape);

        tape->content[tape->cursor] = i;

        print(tape);
        printf("----------------------\n");
    }
    
    free_tape(tape);
}
