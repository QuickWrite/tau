#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PositionData {
    unsigned int line;
    unsigned int line_position;

    char* line_content;
    size_t line_amount;
};

static struct PositionData get_position(FILE *fptr) {
    long position = ftell(fptr);

    unsigned int line_position = 0;
    char c;
    fseek(fptr, -2, SEEK_CUR);

    while(line_position < position && (c = getc(fptr)) != '\n') {
        ++line_position;

        fseek(fptr, -2, SEEK_CUR);
    }

    size_t bufsize = line_position + 10;
    char* buf = calloc(bufsize - 1, sizeof(char));
    
    size_t characters = getline(&buf, &bufsize, fptr);

    rewind(fptr);

    unsigned int lines = 0;
    while ((c = getc(fptr)) != EOF && ftell(fptr) != position) {
        if(c == '\n') ++lines;
    }

    fseek(fptr, position, SEEK_SET);

    return (struct PositionData) {
        .line = lines,
        .line_position = line_position,

        .line_content = buf,
        .line_amount = characters
    };
}

void print_parser_error(FILE* fptr, const char* file_name, const char* error_type, const char* const error_message) {
    fprintf(stderr, "ERROR: %s\n", error_type);
    const struct PositionData position = get_position(fptr);

    fprintf(stderr, "--> %s:%u:%u\n", file_name, position.line + 1, position.line_position + 1);

    // Copied from https://stackoverflow.com/a/5604971
    int linenum_len = snprintf(NULL, 0, "%i", (position.line + 1)) - 1;

    fprintf(stderr, "%*s |\n", linenum_len + 1, "");
    fprintf(stderr, "%u | %s", position.line + 1, position.line_content);

    fprintf(stderr, "%*s | %*s^ %s\n", linenum_len + 1, "", position.line_position, "", error_message);
}
