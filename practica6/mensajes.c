#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#define CICLOS 10

char *pais[3]={"Peru","Bolvia","Colombia"};

// nuestra estructura de mensaje:
// no importa el contenido, 
// sino que haya (o no) un mensaje en la bandeja
struct msgbuf {
  long mtype;
  int mensaje;
};

int msqid; // id del buzon creado
struct msgbuf buf; // nuestro buffer de mensaje
key_t key = 0x1234; // llave del buzón de mensajes


void proceso(int i)
{
	int k;
	int l;

	for(k=0; k<CICLOS; k++)
	{
		// recibe mensaje hasta que haya uno disponible 
		// (rcv es bloqueante)
  	msgrcv(msqid, &buf, sizeof(int), 1, 0);

		// Entrada a la sección crítica
		printf("Entra %s",pais[i]);
		fflush(stdout);
		sleep(rand()%3);
		printf("- %s Sale\n",pais[i]);
		// Salida de la sección crítica

		// al terminar, envía un mensaje para que otro pueda pasar
		msgsnd(msqid, &buf, sizeof(int), 0);

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
	if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
      printf("fallo crear buzon");
      exit(1);
  }

  // establecemos el tipo de mensaje como 1
  // el recieve debe tener el mismo tipo que este
  buf.mtype = 1;

  // establecemos el valor del mensaje
  // es trivial, en realidad no importa
  buf.mensaje = 0;

  // enviamos el primer mensaje
  if (msgsnd(msqid, &buf, sizeof(int), 0) == -1)
  	printf("fallo envio inicial");

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
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
      printf("fallo cierre buzon");
      exit(1);
  }
}