#
################################################################################
#
#   File: makefile
#
#   Makefile for the program ace2bmp
#
#  For valgrind compile with "-g -Og", then run:
#   valgrind -v -v --leak-check=yes --track-origins=yes -s --main-stacksize=200000000 ./ace2bmp AU_kang.ace Au_kang.bmp
#
################################################################################
#
#SHELL      = /bin/sh

#  Specify Compiler and Compiler Options
CC       = gcc
CFLAGS   = -m64 -march=native
CFLAGS  += -O3 #         Level 3 compiler optimisation
#CFLAGS  += -g -Og #    For valgrind use -g and -Og optimisation
CFLAGS  += -Wall -pedantic
CFLAGS  += -Wextra
CFLAGS  += -Wno-unused-variable

CLIB    = -lz

# Which suffixes to process
.SUFFIXES:
.SUFFIXES: .c .h .o

#  Targets that are not the name of a file
.PHONEY: clean

#  Makefile targets:

ace2bmp: ace2bmp.c read_ace.c token_b.c token.c struct.h rgb2bmp.c
	$(CC) ace2bmp.c -o ace2bmp $(CFLAGS) $(CLIB)

clean:
	rm -f ace2bmp *.o *~
