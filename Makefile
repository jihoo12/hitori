CC = clang
TARGET = hitori
SRC = main.c

PKGS = gtk4 gtk4-layer-shell-0

CFLAGS = $(shell pkg-config --cflags $(PKGS)) -Wall -Wextra -g
LIBS = $(shell pkg-config --libs $(PKGS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
