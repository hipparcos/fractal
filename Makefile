EXE=fractal
SOURCES=main.c
DEBUG=-O3
LIBS=-lSDL

CFLAGS=-Wall -c -std=c99 $(DEBUG)
LDFLAGS=-Wall $(LIBS)
CC=gcc
RM=rm -rf
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(EXE)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	$(RM) *.o $(EXE)

rebuild: clean all
