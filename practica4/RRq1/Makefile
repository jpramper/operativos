all: fcfs proceso 

fcfs: virtual_processor.o schedulerRRq1.c
	gcc -o rrq1 schedulerRRq1.c virtual_processor.o 

proceso: proceso.c
	gcc -o proceso proceso.c

virtual_processor.o: virtual_processor.c virtual_processor.h
	gcc -c virtual_processor.c

clean:
	rm rrq1
	rm virtual_processor.o
