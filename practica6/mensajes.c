#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CICLOS 10

char *pais[3]={"Peru","Bolvia","Colombia"};
int *g;

void proceso(int i)
{
	int k;
	int l;

	for(k=0; k<CICLOS; k++)
	{
		// Entrada a la sección crítica
		printf("Entra %s",pais[i]);
		fflush(stdout);
		sleep(rand()%3);
		printf("- %s Sale\n",pais[i]);
		// Salida de la sección crítica
		// Espera aleatoria fuera de la sección crítica
		sleep(rand()%3);
	}

	exit(0);
	// Termina el proceso
}

int main()
{
	int pid;
	int status;
	int args[3];
	int i;
	srand(getpid());

	for(i=0;i<3;i++)
	{
		// Crea un nuevo proceso hijo que ejecuta la función proceso()
		pid=fork();
		if(pid==0)
			proceso(i);
	}

	for(i=0;i<3;i++)
		pid = wait(&status);
}