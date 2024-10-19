#pragma once

#include "lexer.h"
#include "turingmachine.h"

struct TuringMachine* parse(struct Lexer* const lexer);
