#include <signal.h>

#define MAXPROC 3
#define CICLOS 10
#define WAYS 1 // concurrencia de seccion critica

// Macro que incluye el código de la instrucción máquina xchg
#define atomic_xchg(A,B) __asm__ __volatile__( \
			                  " lock xchg %1,%0 ;\n" \
				                           : "=ir" (A) \
			                     : "m" (B), "ir" (A) \
						                                 );


struct COLABLOCK {
    int cola[20];
    int ent;
    int sal;
};

struct SEMAFORO {
    int cnt;
    struct COLABLOCK queue;
};

void mete_a_cola(struct COLABLOCK *q,int proceso);
int cola_vacia(struct COLABLOCK q);
int sacar_de_cola(struct COLABLOCK *q);

void waitsem(struct SEMAFORO *sem, int pid)
{
  printf("entramos a waitsem\n");fflush(stdout);
  if (sem->cnt == 0)
  {
    printf("contador es 0\n");fflush(stdout);
    // pongo el proceso en la cola del semaforo
    mete_a_cola(&sem->queue, pid);
    // bloqueo al proceso
    kill(pid, SIGSTOP);
  }
  else
  {
    // decremento el contador
    sem->cnt--;
  }
}
void signalsem(struct SEMAFORO *sem, int pid)
{
  if (cola_vacia(sem->queue))
  {
    // incremento el contador
    sem->cnt++;
  }
  else
  {
    // saco el siguiente proceso de la cola,
    // y lo desbloqueo (mando a listos)
    kill(sacar_de_cola(&sem->queue), SIGCONT);
  }
}
void initsem(struct SEMAFORO *sem,int ways)
{
  // inicializa el numero de procesos que pueden
  // entrar a la zona critica al mismo tiempo
  sem->cnt = ways;
}

void mete_a_cola(struct COLABLOCK *q,int proceso)
{
    if(proceso == -1 )
      return;

    q->cola[q->ent]=proceso;
    q->ent++;
    if(q->ent>19)
       q->ent=0;
}

int cola_vacia(struct COLABLOCK q)
{
    if(q.ent==q.sal)
        return(1);
    else
        return(0);
}

int sacar_de_cola(struct COLABLOCK *q)
{
    int proceso;

    proceso=q->cola[q->sal];
    q->sal++;
    if(q->sal>19)
        q->sal=0;
    return(proceso);
}
