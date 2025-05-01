CFLAGS = -O3 -g
LINKER = -Xlinker -Ttext -Xlinker 80140000

PROG = main.exe
OBJS = main.o yario.o pad.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LINKER) -o $@ $?

main.o: main.c 
	$(CC) $(CFLAGS) -c main.c

yario.o: yario.c yario.h
	$(CC) $(CFLAGS) -c yario.c

pad.o: pad.c pad.h
	$(CC) $(CFLAGS) -c pad.c

rebuild: 
	make clean
	echo ---------- building ------
	make all 

psx.exe: $(PROG) main.exe 
	yarexe AUTO

run: psx.exe
	nopsx psx.exe

clean:
	$(RM) $(PROG)
	$(RM) $(OBJS)