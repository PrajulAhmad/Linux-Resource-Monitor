CC=gcc
CFLAGS=-Wall -Wextra -O2

SRC = main.c analyzer.c reader.c logger.c
OBJ = $(SRC:.c=.o)
TARGET = monitor

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c monitor.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
