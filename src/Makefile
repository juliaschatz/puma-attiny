IDIR =../include
CC=avr-gcc
CFLAGS=-I$(IDIR) -mmcu=attiny20 -Os -Wall -g

ODIR=../build
LDIR =../lib

LIBS=

ELF_FILE=../build/image.elf
HEX_FILE=../build/image.hex

_DEPS = main.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: | clean build flash

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build: $(OBJ)
	$(CC) -o $(ELF_FILE) $^ $(CFLAGS) $(LIBS)

hex:
	avr-objcopy -j .text -j .data -O ihex $(ELF_FILE) $(HEX_FILE)

flash:
	avrdude -p attiny20 -c avrftdi -U flash:w:$(ELF_FILE)

.PHONY: clean build flash hex

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 