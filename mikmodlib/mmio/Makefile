# MMIO Library Makefile for Linux
# Divine Entertainment Game Programming Pack
#

##########################
## User settable macros ##
##########################
Compiler          = psp-gcc
Librarian         = psp-ar
Compiler_Options  = -G0 -O2 -I. -I../mikmod -I../include -g
CC = $(Compiler)

Lib_file          = ../lib/libmmio.a

Object_files      = mmio.o mmalloc.o mmerror.o

####################
## Makefile rules ##
####################

$(Lib_file): $(Object_files)
	$(Librarian) r $(Lib_file) $(Object_files)

%.o: %.c
	$(CC) -c $(Compiler_Options) $<

clean:
	rm -f *.o
	rm -f $(Lib_file)
