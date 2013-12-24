C_FILES = $(wildcard /usr/local/src/mongo/*.c)
OBJ_FILES = $(patsubst /usr/local/src/mongo/%.c, obj/%.o, $(C_FILES))
.DEFAULT_GOAL = mongo

obj:
	mkdir obj

bin:
	mkdir bin

obj/%.o: /usr/local/src/mongo/%.c
	gcc -g --std=c99 -D_POSIX_SOURCE -DMONGO_HAVE_STDINT -fPIC -Wall -c -o $@ $<

mongo: obj bin $(OBJ_FILES)
	gcc -g -I/usr/local/src/ --std=c99 -fPIC -Wall -c -o obj/mongodb.o src/mongodb.c
	gcc -g -shared -o bin/mongodb.ndll -lneko obj/*.o
