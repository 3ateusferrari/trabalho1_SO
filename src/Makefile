CC=gcc
CFLAGS=-Wall -pthread -I.

SRCS=main.c helicoptero.c bateria.c foguete.c motor.c utils.c
OBJS=$(SRCS:.c=.o)

all: jogo

jogo: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o jogo 