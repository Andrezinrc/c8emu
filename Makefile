CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LIBS = -lSDL2

TARGET = mychip8
BUILD_DIR = build

SRCS = main.c cpu.c config.c SDL/video.c SDL/audio.c SDL/keypad.c

OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(BUILD_DIR)
