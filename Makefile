EXE=fractal
SOURCES=main.c debug.c fractal.c event.c generator/*
DEBUG=-O3 -DDEBUG
LIBS=-lSDL -lm

CFLAGS=-Wall -c -std=c99 $(DEBUG)
LDFLAGS=-Wall -zmuldefs $(LIBS)
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
