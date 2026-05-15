CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LIBS = -lSDL2

TARGET = mychip8
SRCS = main.c cpu.c display.c
OBJS = main.o cpu.o display.o

ll: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

main.o: main.c cpu.h
	$(CC) $(CFLAGS) -c main.c -o main.o

cpu.o: cpu.c cpu.h
	$(CC) $(CFLAGS) -c cpu.c -o cpu.o

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c -o  display.o

clean:
	rm -f $(TARGET) $(OBJS)
