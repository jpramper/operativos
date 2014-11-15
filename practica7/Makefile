all: procesos procesosVM swap

procesos: procesos.c mmu.o pagefault.o
	cc -o procesos procesos.c mmu.o pagefault.o

procesosVM: procesos.c mmu.o pagefaultVM.o
	cc -o procesosVM procesos.c mmu.o pagefaultVM.o

pagefault.o: pagefault.c mmu.h
	cc -c pagefault.c

pagefaultVM.o: pagefaultVM.c mmu.h
	cc -c pagefaultVM.c

mmu.o: mmu.c semaphores.h mmu.h
	cc -c mmu.c

swap:
	./createswap
