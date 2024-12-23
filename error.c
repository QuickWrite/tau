#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

#define calc_chars(number) (int)((ceil(log10(number))+1)*sizeof(char))

void print_parser_error(FILE* fptr, const char* file_name, char* error_type, const char* const error_message) {
    fprintf(stderr, "ERROR: %s\n", error_type);
    struct PositionData position = get_position(fptr);

    fprintf(stderr, "--> %s:%u:%u\n", file_name, position.line + 1, position.line_position + 1);

    int chars = calc_chars(position.line + 2);
    chars = chars > 1 ? chars : 2;

    char start_whitespace[chars];
    memset(start_whitespace, ' ', chars);
    start_whitespace[chars - 1] = '\0';

    fprintf(stderr, "%s |\n", start_whitespace);
    fprintf(stderr, "%u | %s", position.line + 1, position.line_content);
    
    char position_whitespace[position.line_position + 1];
    memset(position_whitespace, ' ', position.line_position);
    position_whitespace[position.line_position] = '\0';

    fprintf(stderr, "%s | %s^ %s\n", start_whitespace, position_whitespace, error_message);
}
