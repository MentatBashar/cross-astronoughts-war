CC = gcc
CCFLAGS = -std=c99 -lm -Wall -Werror -Wpedantic
AL = $$(pkg-config allegro-5 allegro_main-5 allegro_image-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 --libs --cflags)

SOURCE = caw.c
BIN = caw.exe

all: caw

clean:
	-rm $(BIN)

caw: caw.c caw.h
	${CC} ${SOURCE} ${CCFLAGS} ${AL} -o ${BIN}
