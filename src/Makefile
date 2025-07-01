CC=gcc
CFLAGS=-Wall -I. -I/mingw64/include/SDL2
LIBS=-L/mingw64/lib -lSDL2main -lSDL2 -lpthread

SRCS=main.c helicoptero.c bateria.c foguete.c motor.c utils.c graphics.c
OBJS=$(SRCS:.c=.o)

all: jogo

jogo: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o jogo 