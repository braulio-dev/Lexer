CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

SRCS = main.c lexer.c parser.c
OBJS = $(SRCS:.c=.o)
TARGET = java_parser

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean 