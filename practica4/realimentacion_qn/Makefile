all: fcfs proceso 

fcfs: virtual_processor.o schedulerRqn.c
	gcc -o rqn schedulerRqn.c virtual_processor.o -lm

proceso: proceso.c
	gcc -o proceso proceso.c

virtual_processor.o: virtual_processor.c virtual_processor.h
	gcc -c virtual_processor.c


clean:
	rm rqn
	rm virtual_processor.o
