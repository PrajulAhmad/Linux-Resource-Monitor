CC = gcc
CFLAGS = -Wall -Wextra
TARGET = monitor
OBJS = main.o reader.o analyzer.o logger.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c monitor.h
	$(CC) $(CFLAGS) -c main.c
reader.o: reader.c monitor.h
	$(CC) $(CFLAGS) -c reader.c
analyzer.o: analyzer.c monitor.h
	$(CC) $(CFLAGS) -c analyzer.c
logger.o: logger.c monitor.h
	$(CC) $(CFLAGS) -c logger.c

clean:
	rm -f $(OBJS) $(TARGET)
