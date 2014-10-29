#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/shm.h>
#include <semaphore.h>

#define CICLOS 10

sem_t *sem;

char *pais[3]={"Peru","Bolvia","Colombia"};
int *g;

void proceso(int i)
{
	int k;
	int l;

	for(k=0; k<CICLOS; k++)
	{
		// waitsem linux
		sem_wait(sem);

		// Entrada a la sección crítica
		printf("Entra %s",pais[i]);
		fflush(stdout);
		sleep(rand()%3);
		printf("- %s Sale\n",pais[i]);
		// Salida de la sección crítica

		// signalsem linux
		sem_post(sem);

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

	int shmid; // memoria compartida

	// initsem linux
	shmid = shmget(0x1234,sizeof(sem),0666|IPC_CREAT);

	if(shmid==-1)
	{
		perror("Error en la memoria compartida\n");
		exit(1);
	}

	// Declarar el semaforo en memoria compartida
	sem = shmat(shmid,NULL,0);

	if(sem == NULL)
	{
		perror("Error en el shmat\n");
		exit(2);
	}

	// inciializa el semaforo
	if (sem_init(sem, 1, 1) != 0)
	{
		perror("Error en el init_sem\n");
		exit(3);
	}

	for(i=0;i<3;i++)
	{
		// Crea un nuevo proceso hijo que ejecuta la función proceso()
		pid=fork();
		if(pid==0)
			proceso(i);
	}

	for(i=0;i<3;i++)
		pid = wait(&status);

	// Eliminar la memoria compartida
	shmdt(sem);
}