CC=$(PREFIX)gcc
STRIP=$(PREFIX)strip

SRC=pspack.c index.c asprintf.c minilzo.c util.c fs.c ansicolor-w32.c colors.c
EXE=pspack.exe
CFLAGS=-Wall -O2

OBJ=$(SRC:%.c=%.o)

all : $(EXE)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE) : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)
	$(STRIP) $@

clean:
	-rm -f $(OBJ) $(EXE)
