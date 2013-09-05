CFLAGS = -g -std=c99 -Wall
LDFLAGS = -lreadline
TARGET = group1
%: all

all: main.c
	$(CC) $(CFLAGS) $< -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
