CC = gcc

CFLAGS = -Wall -Wextra -O2

TARGET = septica_game

SRCS = main.c septica.c interface.c ai.c

OBJS = $(SRCS:.c=.o)

HEADERS = septica.h interface.h ai.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC)	$(CFLAGS)	-o	$(TARGET)	$(OBJS)

%.o: %.c $(HEADERS)
	$(CC)	$(CFLAGS)	-c	$<	-o	$@

clean:
	rm -f ./path/file-f	$(OBJS)	$(TARGET)	$(TARGET).exe

.PHONY: all clean
 
