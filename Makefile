CC=$(PREFIX)gcc
STRIP=$(PREFIX)strip

SRC=pspack.c index.c util.c fs.c colors.c
SRC_LIB=asprintf.c minilzo.c ansicolor-w32.c

EXE=pspack.exe
CFLAGS=-Wall -O2

OBJ=$(SRC:%.c=%.o)
OBJ_LIB=$(SRC_LIB:%.c=%.o)

all : $(EXE)

$(EXE) : $(OBJ_LIB) $(OBJ)
	@echo "LINK $@ <- $(OBJ) $(OBJ_LIB)"
	@$(CC) $(LDFLAGS) -o $@ $(OBJ) $(OBJ_LIB)
	@$(STRIP) $@

# we dont want lots of errors for "library" files
$(OBJ_LIB): CFLAGS := -O2

%.o : %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJ) $(OBJ_LIB) $(EXE)
