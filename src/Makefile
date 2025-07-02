CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -pthread
LIBS = -lSDL2 -lSDL2_ttf -lm -pthread

TARGET = jogo
SOURCE = tudo_em_um.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run