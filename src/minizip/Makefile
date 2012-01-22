AR=ar
CC=cc
CFLAGS=-O -I../.. $(MINIZIP_CFLAGS)
RANLIB=ranlib

UNZ_OBJS = miniunz.o unzip.o ioapi.o -lz
ZIP_OBJS = minizip.o zip.o   ioapi.o -lz
MIN_OBJS = unzip.o zip.o ioapi.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c

#all: miniunz minizip
all: miniunz minizip libminizip.a

miniunz:  $(UNZ_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UNZ_OBJS)

minizip:  $(ZIP_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ZIP_OBJS)

libminizip.a: $(MIN_OBJS)
	$(AR) r $@ $(MIN_OBJS)
	$(RANLIB) $@

#test:	miniunz minizip
#	./minizip test readme.txt
#	./miniunz -l test.zip
#	mv readme.txt readme.old
#	./miniunz test.zip

clean:
#	/bin/rm -f *.o *~ minizip miniunz
	/bin/rm -f *.o *~ libminizip.a

