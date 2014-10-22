#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "narcos.h"

char *pais[3]={"Peru","Bolvia","Colombia"};

struct SEMAFORO *sem;
int *gwait;
int *gsig;

void proceso(int i, int pid)
{
  int k;
	int lwait;
	int lsignal;

  for(k=0;k<CICLOS;k++)
  {
    lwait=1;
		//printf("omaiga %s, g=%d, l=%d\n", pais[i], *gwait, lwait);
		fflush(stdout);
    do { atomic_xchg(lwait,*gwait); } while(lwait!=0);
		// comprobacion del semaforo
		waitsem(sem, pid);
		*gwait = 0;

		// <zona critica>
    printf("Entra %s",pais[i]);
    fflush(stdout);
    //sleep(5/*rand()%3*/);
    printf("- %s Sale\n",pais[i]);
		fflush(stdout);
		// </zona critica>

		lsignal=1;
		do { atomic_xchg(lsignal,*gsig); } while(lsignal!=0);
		// Llamada waitsignal
		signalsem(sem, pid);
		*gsig = 0;
		// Espera aleatoria fuera de la sección crítica
		sleep(rand()%3);
  }
  exit(0); // Termina el proceso
}


int main()
{
  int pid;
  int status;
  int shmid, shmidgwait, shmidgsig;
  int args[3];
  int i;
	printf("inicio\n");
	fflush(stdout);
  // definir la memoria compartida
  shmid=shmget(0x1234,sizeof(sem),0666|IPC_CREAT);
	shmidgwait=shmget(0x1250,sizeof(gwait),0666|IPC_CREAT);
	shmidgsig=shmget(0x1275,sizeof(gsig),0666|IPC_CREAT);

	printf("pase el shmget \n");
	fflush(stdout);
  if(shmid==-1 || shmidgwait==-1 || shmidgsig==-1)
  {
    perror("Error en la memoria compartida\n");
    exit(1);
  }

	// Declarar el semaforo en memoria compartida
  sem = shmat(shmid,NULL,0);
	// declarar las exchanges
	gwait = shmat(shmidgwait,NULL,0);
	gsig =shmat(shmidgsig,NULL,0);

  if(sem == NULL || gwait == NULL || gsig == NULL)
  {
    perror("Error en el shmat\n");
    exit(2);
  }

	// inciializa el semaforo y los xchnges
  initsem(sem, WAYS);
  *gwait = 0;
  *gsig = 0;
	printf("iniciamos nuestro semanforo, gwait=%d, gsig=%d\n", *gwait, *gsig);
	fflush(stdout);
  srand(getpid()); // random seed con el pid del padre

  for(i=0;i<3;i++)
  {
    // Crea un nuevo proceso hijo que ejecuta la función proceso()
    pid=fork();
    if(pid==0){
			printf("\n entramos a proceso: %d", i);
			fflush(stdout);
			proceso(i, pid);
		}

  }

  for(i=0;i<3;i++)
    pid = wait(&status);

  // Eliminar la memoria compartida
	shmdt(sem);
	shmdt(gwait);
	shmdt(gsig);
}


/*
void proceso(int i)
{
  int k;
  for(k=0;k<CICLOS;k++)
  {
    // Llamada waitsem implementada en la parte 3
    waitsem(sem);
    printf("Entra %s ",pais[i]);
    fflush(stdout);
    sleep(rand()%3);
    printf("- %s Sale\n",pais[i]);
    // Llamada waitsignal implementada en la parte 3
    signalsem(sem);
    // Espera aleatoria fuera de la sección crítica
    sleep(rand()%3);
  }
  exit(0); // Termina el proceso
}

int main()
{
    …
    …
    // Incializar el contador del semáforo en 1 una vez que esté
    // en memoria compartida, de manera que solo a un proceso se le
    // permitirá entrar a la sección crítica
}
*/
