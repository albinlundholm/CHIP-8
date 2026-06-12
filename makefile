CC = gcc
CFLAGS = -Wall -Wextra $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -mconsole
SRC = src/main.c src/chip8.c
chip8: $(SRC)
	$(CC) $(CFLAGS) -o chip8 $(SRC) $(LDFLAGS)

clean:
	rm -f chip8.exe chip8
