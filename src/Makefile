files=test-bw.c cfg.c db.c
objects=../obj/test-bw.o ../obj/db.o ../obj/cfg.o

LIBS=-lpthread -lfcgi -lmysqlclient -lmemcached -lpcre2-8
CFLAGS= -std=c99 -O2 -Wall -Wextra -ftree-vectorize -c
TARGET_OUTPUT_FILE=../bin/test-bw

GCC=/usr/bin/gcc $(CFLAGS)
LINK=/usr/bin/gcc $(LIBS)

all : $(objects)
	$(LINK) $(objects) -s -o $(TARGET_OUTPUT_FILE)

../obj/db.o : db.c db.h cfg.h
	$(GCC) db.c -o ../obj/db.o

../obj/cfg.o : cfg.c cfg.h
	$(GCC) cfg.c -o ../obj/cfg.o

../obj/test-bw.o : test-bw.c cfg.h
	$(GCC) test-bw.c -o ../obj/test-bw.o

.PHONY : clean

clean : 
	rm $(objects)

