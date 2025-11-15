CC = gcc
CFLAGS = -Isrc/include -Wall
LDFLAGS = -lraylib -lm
TARGET = gridgame
SRC = $(wildcard src/*.c) $(wildcard src/events/*.c)

.PHONY: all install clean

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	sudo mv $(TARGET) /usr/bin/

clean:
	rm -f $(TARGET)
