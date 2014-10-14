#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "virtual_processor.h"

extern struct PROCESO proceso[];
extern struct COLAPROC listos,bloqueados;
extern int tiempo;
extern int pars[];

void push(int nuevo_id);
int pop();

#define TRUE 1
#define FALSE 0

// =============================================================================
// ESTE ES EL SCHEDULER
// ============================================================================

int scheduler(int evento)
{
    printf("    <Scheduler>\n");
    int cambia_proceso = FALSE; // bandera de cambio de proceso
    int prox_proceso_a_ejecutar = pars[1]; // pid del proceso en ejecucion

    switch(evento)
    {
      case 0:
        printf("        EVENTO: TIMER, ejecucion actual: %d\n",pars[0]);



        if(cola_vacia(listos))
          printf("        - Cola vacia\n");
        else
        {
            printf("        - Cola no vacia\n");
            // lo que estaba en ejecucion se mete a listos
            proceso[pars[1]].estado = LISTO;
            push(pars[1]);
            // y se hace push

            cambia_proceso = TRUE;
        }

        break;

      ///////////////////////////////////////////////
      case 1:
        printf("        EVENTO: SOLICITA_E_S, ejecucion actual: %d\n",pars[0]);

        // se mete a la cola de bloqueados y se mete a cola de bloqueados
        proceso[pars[1]].estado = BLOQUEADO;
        mete_a_cola(&bloqueados,pars[1]);
        cambia_proceso = TRUE;

        break;

      ///////////////////////////////////////////////
      case 2:
        printf("        EVENTO: TERMINA_E_S, ejecucion actual: %d\n",pars[0]);

        // se desbloquea el proceso y se saca de la cola de bloqueados
        proceso[pars[0]].estado = LISTO;
        push(sacar_de_cola(&bloqueados));

        break;

      ///////////////////////////////////////////////
      case 3:
        printf("        EVENTO: PROCESO_NUEVO, ejecucion actual: %d\n",pars[0]);
        //if(!pars[1]== -1){
          int p= proceso[pars[0]].prioridad;
          push(pars[0]);
          proceso[pars[0]].estado = LISTO;// Agregar el nuevo proceso a la cola de listos
          proceso[pars[0]].prioridad = p;// Asegura que la prioridad no haya cambiado en el push
          if(tiempo==0)
            cambia_proceso = TRUE;
        //}

        break;

      ///////////////////////////////////////////////
      case 4:
        printf("        EVENTO: PROCESO_TERMINADO, ejecucion actual: %d\n",pars[0]);

        proceso[pars[0]].estado = TERMINADO;
        cambia_proceso = TRUE; // se pide cambio de proceso

        break;

    }


    if(cambia_proceso)
    {
        printf("        - Se solicita cambio de proceso\n");

        // Si la cola no esta vacia obtener de la cola el siguiente proceso listo
        if(!cola_vacia(listos))
        {
            prox_proceso_a_ejecutar=pop();
            proceso[prox_proceso_a_ejecutar].estado=EJECUCION;
            cambia_proceso=0;
        }
        else
        {
            printf("        - no hay listos\n");
            prox_proceso_a_ejecutar=NINGUNO;
        }
    }

    printf("        PROCESO siguiente: %d\n",prox_proceso_a_ejecutar);
    printf("    </Scheduler>\n");
    return(prox_proceso_a_ejecutar);
}

void push(int nuevo_id)
{
  /*si hay algo atras de "el" */
  /*y su prioridad es menor a el limite (n procesos) */
  if(listos.sal>0 && proceso[nuevo_id].prioridad < PRIORIDADES)
    proceso[nuevo_id].prioridad++; /* se incrementa la prioridad*/


  listos.cola[listos.sal]= nuevo_id;  /*metes el proceso a la ultima posicion de la cola*/
  listos.sal ++; /*incrementas el apuntador de la cola*/
}

int pop()
{
  int             menor_i  = 0; /*indice del menor en la cola*/
  int             answer   = NINGUNO; /*ID proceso a buscar con menor prioridad*/
  struct PROCESO  menor_proceso = proceso[listos.cola[menor_i]];/* objeto proceso */
  /*iniciamos apuntando al primero*/
  int i;
  for(i=0;i<listos.sal;i++)
  {
    struct PROCESO temp= proceso[listos.cola[i]]; /*temporal para hacer comparaciones*/
    if(temp.prioridad < menor_proceso.prioridad)
      {
        menor_i=i;
        menor_proceso= proceso[listos.cola[menor_i]];
      }
  }
  /*despues del FOR regresa el primer objeto con menor prioridad*/

  //guarda resultado
  answer=listos.cola[menor_i];

  //recorre
  for(i=menor_i;i<(listos.sal-1);i++)/*reconcatena las 2 listas*/
    listos.cola[i]=listos.cola[i+1];

  listos.sal--; /*disminuimos tama;o de lista*/

  return answer;
}
