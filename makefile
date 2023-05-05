CC = gcc
CCFLAGS = -std=c99 -lm -Wall -Werror -Wpedantic
AL = $$(pkg-config allegro-5 allegro_main-5 allegro_image-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 --libs --cflags)

SOURCE = src/caw.c
BIN = bin/caw

all: caw

clean:
	-rm $(BIN)

caw: src/caw.c src/caw.h
	mkdir -p bin/
	${CC} ${SOURCE} ${CCFLAGS} ${AL} -o ${BIN}
