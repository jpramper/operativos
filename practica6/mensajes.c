#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>

// http://uw714doc.sco.com/en/SDK_sysprog/_Example_Program3.html
// http://students.mimuw.edu.pl/SO/Linux/Kod/include/linux/msg.h.html

#define CICLOS 10

char *pais[3]={"Peru","Bolvia","Colombia"};
int *g;

int msqid; // id del buzon creado
struct msqid_ds buf;

struct msgbuf {
	long mtype;
	char mtext[8192];
} rcvBuf, sndBuf, *message;

void proceso(int i)
{
	int k;
	int l;

	for(k=0; k<CICLOS; k++)
	{
		// mensaje


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

	// creamos el buzón de mensajes
	if ((msqid = msgget(0x1234, 0666|IPC_CREAT)) == -1)
	{
		perror("msgget: msgget failed");
		exit(1);
	}

	if( msgctl( msqid, IPC_STAT, &buf) == -1)
  {
		perror("msgget: IPC_STAT failed");
		exit(2);
	} 

	// apuntamos a el mensaje a enviar
	message = &sndBuf;
	// ponemos el primer mensaje en el buzon
	message->mtype = 1;
	if (msgsnd(msqid, (const void*) message, 0, 0) == -1)
		printf("omg");

	message = &rcvBuf;

	msgrcv(msqid, (void*) message, 0, 0, 0);
		printf("rcv1");

	msgrcv(msqid, (void*) message, 0, 0, 0);
		printf("rcv2");

	msgrcv(msqid, (void*) message, 0, 0, 0);
		printf("rcv3");

	//comprobación de estructura
	puts("ola");
	printf("\n%d\n", buf.msg_qnum);

	for(i=0;i<3;i++)
	{
		// Crea un nuevo proceso hijo que ejecuta la función proceso()
		pid=fork();
		if(pid==0)
			proceso(i);
	}

	for(i=0;i<3;i++)
		pid = wait(&status);

	// cerramos el buzón
	if (msgctl(msqid, IPC_RMID, &buf) == -1)
		perror("msgctl: msgctl failed");
}