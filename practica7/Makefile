all: procesos swap

procesos: procesos.c mmu.o pagefault.o
	cc -o procesos procesos.c mmu.o pagefault.o

pagefault.o: pagefault.c mmu.h
	cc -c pagefault.c

mmu.o: mmu.c semaphores.h mmu.h
	cc -c mmu.c

swap:
	./createswap
