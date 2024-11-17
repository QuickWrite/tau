build:
	cc main.c tape.c turingmachine.c lexer.c parser.c error.c -Wall -Wextra -lm -o tau
