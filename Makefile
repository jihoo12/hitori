CC = clang
TARGET = hitori
SRC = $(wildcard src/*.c)

PKGS = gtk4 gtk4-layer-shell-0

CFLAGS = $(shell pkg-config --cflags $(PKGS)) -Isrc -Wall -Wextra -g
LIBS = $(shell pkg-config --libs $(PKGS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
