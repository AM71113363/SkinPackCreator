# Project: AM71113363

CC   = gcc.exe -s
WINDRES = windres.exe
RES  = main.res
OBJ  = main.o crc32.o myzip.o uuid.o $(RES)
LINKOBJ  = main.o crc32.o myzip.o uuid.o $(RES)
LIBS =  -L"C:/Dev-Cpp/lib" -mwindows  -g3 
INCS =  -I"C:/Dev-Cpp/include" 
BIN  = SkinPackCreator.exe
CFLAGS = $(INCS)   -g3
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before SkinPackCreator.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o "SkinPackCreator.exe" $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

crc32.o: crc32.c
	$(CC) -c crc32.c -o crc32.o $(CFLAGS)

myzip.o: myzip.c
	$(CC) -c myzip.c -o myzip.o $(CFLAGS)

uuid.o: uuid.c
	$(CC) -c uuid.c -o uuid.o $(CFLAGS)

main.res: main.rc 
	$(WINDRES) -i main.rc --input-format=rc -o main.res -O coff 
