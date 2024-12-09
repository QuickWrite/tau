#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "turingmachine.h"
#include "parser.h"

struct Arguments {
    size_t view_width;
    size_t max_iter;
};

void print_machine(const struct TuringMachine* const machine, const size_t view_width){
    size_t whitespace_len = 0;

    const struct Tape* const tape = &machine->tape;

    long top = view_width % 2 == 1 ? tape->cursor + (view_width / 2) + 1 : tape->cursor + (view_width / 2);


    printf("..., ");
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

// Copied from https://stackoverflow.com/a/12923949
typedef enum {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

/* Convert string s to int out.
 *
 * @param[out] out The converted int. Cannot be NULL.
 *
 * @param[in] s Input string to be converted.
 *
 *     The format is the same as strtol,
 *     except that the following are inconvertible:
 *
 *     - empty string
 *     - leading whitespace
 *     - any trailing characters that are not part of the number
 *
 *     Cannot be NULL.
 *
 * @param[in] base Base to interpret string in. Same range as strtol (2 to 36).
 *
 * @return Indicates if the operation succeeded, or why it failed.
 */
str2int_errno str2int(int *out, const char *s, int base) {
    char *end;
    if (s[0] == '\0' || isspace(s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = l;
    return STR2INT_SUCCESS;
}

void print_help(void) {
    printf(
        "\033[1;4mTAU Help\033[0m\n\n"
        "\033[3;4mGeneral command syntax:\033[0m\n"
        "  ./tau <filename>\n\n"
        "\033[3;4mFlags:\033[0m\n"
        "  --help          Prints this help message.\n"
        "  --view-width    Sets the amount of cells that are being printed.\n"
        "                  \033[2m(default: 9)\033[0m\n"
        "  --max-iter      Sets the maximum amount of iterations the Turing Machine can do.\n"
        "                  \033[2m(default: 5000)\033[0m\n"
    );
}

int check_long(struct Arguments* arguments, const int argc, const char** const argv) {
    if(argc == 0) {
        // error
        return 0;
    }

    if(strcmp(&argv[0][2], "help") == 0) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    int second;
    if(str2int(&second, argv[1], 10) != STR2INT_SUCCESS) {
        fprintf(stderr, "\033[31mThe content of the flag has to be a number.\033[0m\n");
        return 0;
    }

    if(strcmp(&argv[0][2], "view-width") == 0) {
        if(second < 5) {
            fprintf(stderr, "\033[31mThe view cannot be smaller than 5.\033[0m\n");
            return 0;
        }
        arguments->view_width = second;

        return 1;
    }

    if(strcmp(&argv[0][2], "max-iter") == 0) {
        if(second < 1) {
            fprintf(stderr, "\033[31mThe iterations cannot be less than 1.\033[0m\n");
            return 0;
        }

        arguments->max_iter = second;
        return 1;
    }

    fprintf(stderr, "\033[31mUnknown flag: \033[32m'%s'\033[31m.\033[0m\n", argv[0]);
    return 0;
}

int parse_arguments(struct Arguments* arguments, const int argc, const char** const argv) {
    // This function currently only supports value flags
    // and not boolean flags. This could be added later on.
    for(int i = 2; i < argc; ++i) {
        if(argv[i][0] == '-') {
            // Check if long
            if(argv[i][1] == '-') {
                if(!check_long(arguments, argc - i, &argv[i])) {
                    // Error handling
                    return 0;
                }
                ++i;
                continue;
            }

            fprintf(stderr, "\033[31mCurrently short flags are not supported.\033[0m\n");
            return 0;
        } else {
            fprintf(stderr, "\033[31mUnknown element %s, maybe try removing it.\033[0m\n", argv[i]);
            return 0;
        }
    }
    return 1;
}

int main(const int argc, const char** const argv) {
    if(argc < 2) {
        fprintf(stderr, "A file path has to be provided: `tau <path>`\n");
        return 1;
    }

    struct Arguments arguments = {
        .view_width = 9,
        .max_iter = 5000
    };

    if(strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    if(argc > 2 && !parse_arguments(&arguments, argc, argv)) {
        fprintf(stderr, "Something went wrong: Aborting.\n");
        return 10;
    }
    
    struct TuringMachine* machine = parse(argv[1]);

    printf("\033[1;4mExecution sequence:\033[0m\n\n");

    size_t count = 0;
    while(machine->state != NULL) {
        printf("State: %s\n", machine->state->name);
        print_machine(machine, arguments.view_width);
        next_state(machine);
        printf("\n");
        ++count;
        if(count >= arguments.max_iter) {
            fprintf(stderr, "\033[31mError: Maximum iterations reached: %zu. Stopping.\033[0m\n", arguments.max_iter);
            return 2;
        }
    }
    printf("State: HALT\n");
    print_machine(machine, arguments.view_width);
}
