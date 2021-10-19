# Makefile for C-Minus Scanner
# ./lex/tiny.l --> ./cminus.l

CC = gcc 

CFLAGS = 

OBJS = main.o util.o scan.o 


.PHONY: all clean
all: cminus_cimpl

clean:
	-rm -vf cminus_cimpl *.o

cminus_cimpl: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) 

main.o: main.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c -o $@ $<

scan.o: scan.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c -o $@ $<

util.o: util.c globals.h util.h
	$(CC) $(CFLAGS) -c -o $@ $<

