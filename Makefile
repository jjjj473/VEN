CC = gcc
CFLAGS := -Wall $(shell pkg-config --cflags gtk+-3.0 gtksourceview-4)
LDFLAGS := $(shell pkg-config --libs gtk+-3.0 gtksourceview-4)
SRC = src/ven.c
BIN = ven

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(LDFLAGS)

clean:
	rm -f $(BIN)

.PHONY: all clean
