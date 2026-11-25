HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)

CFLAGS = -Wall -Wextra

build: $(HEADERS) $(SOURCES)
	cc $(SOURCES) $(CFLAGS) -lm -o tau

release: $(HEADERS) $(SOURCES)
	cc $(SOURCES) $(CFLAGS) -lm -o tau -O3 -static

clean:
	rm -f *.o
	rm -f tau
