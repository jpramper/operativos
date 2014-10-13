all: fcfs proceso 

fcfs: virtual_processor.o schedulerRq1.c
	gcc -o rq1 schedulerRq1.c virtual_processor.o 

proceso: proceso.c
	gcc -o proceso proceso.c

virtual_processor.o: virtual_processor.c virtual_processor.h
	gcc -c virtual_processor.c

clean:
	rm rq1
	rm virtual_processor.o
