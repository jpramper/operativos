all: fcfs proceso 

fcfs: virtual_processor.o schedulerFCFS.c
	gcc -o fcfs schedulerFCFS.c virtual_processor.o 

proceso: proceso.c
	gcc -o proceso proceso.c

virtual_processor.o: virtual_processor.c virtual_processor.h
	gcc -c virtual_processor.c


clean:
	rm fcfs
	rm virtual_processor.o
