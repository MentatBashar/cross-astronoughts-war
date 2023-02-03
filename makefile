CC = gcc
CCFLAGS = -std=c99 -lm -Wall -Werror -Wpedantic
AL = $$(pkg-config allegro-5 allegro_main-5 allegro_font-5 allegro_image-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 --libs --cflags)

SOURCE = space_wars.c
BIN = space_wars.exe

all: space_wars

clean:
	-rm $(BIN)

space_wars: space_wars.c space_wars.h
	${CC} ${SOURCE} ${CCFLAGS} ${AL} -o ${BIN}
