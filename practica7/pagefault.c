#include <stdio.h>
#include "mmu.h"

#define RESIDENTSETSIZE 24

extern char *base;
extern int framesbegin;
extern int idproc;
extern int systemframetablesize;
extern int processpagetablesize;

extern struct SYSTEMFRAMETABLE *systemframetable;
extern struct PROCESSPAGETABLE processpagetable[];

int getfreeframe();
int getOldestUsed();

// Rutina de fallos de página

int pagefault(char *vaddress)
{
    // apuntador a swap
    FILE *swapFile;
    // contador de marcos asignados al proceso
    int marcosAsignados;

    // hablando del sistema
    int marcoNuevo = -1;
    int marcoViejo;

    // hablando del proceso
    int paginaNueva;
    int paginaVieja;

    // Calcula el indice de la página del proceso
    paginaNueva = (int) vaddress >> 12;

    // Cuenta los marcos asignados al proceso
    marcosAsignados = countframesassigned();
  
    // Busca el indice de un marco libre en el sistema
    //if(marcosAsignados < 2)
        marcoNuevo = getfreeframe();
    printf("HOLAAAAAA %d \n",marcoNuevo);
    fflush(stdout);
    // si no hay marcos libres
    if(marcoNuevo == -1)
    {
        // algoritmo de remplazo
        // ---------------------
        // busca el indice de la pagina mas vieja del proceso
        printf("ya no cupe porque no hay memoria suficiente pinchi taca;o :B \n");
        fflush(stdout);
        paginaVieja = getOldestUsed();

        // escribimos la pagina vieja en memoria, si fue modificada
        if (processpagetable[paginaVieja].modificado)
        {
            swapFile = fopen("swap","wb");
            // busca la posicion de la página vieja en swap
            fseek(swapFile, processpagetable[paginaVieja].framenumber * PAGESIZE,0);
            // escribe el valor actual en swap
            fwrite(systemframetable[processpagetable[paginaVieja].framenumber].paddress,1,PAGESIZE,swapFile);

            fclose(swapFile);
        }

        // si la pagina es nueva, asignale su lugar en swap
        printf("mi proceso en su numero loco es %d\n", processpagetable[paginaNueva].framenumber);
        fflush(stdout);

        if (processpagetable[paginaNueva].framenumber != -1)
        {
            // haces el algor intercambio
            swapFile = fopen("swap","rb");

            // busca la posicion de la página actual en swap
            fseek(swapFile, processpagetable[paginaNueva].framenumber * PAGESIZE,0);
        }
        else
        {
            // hay que darle espacio en swap
            int indice = 0;
            swapFile = fopen("swap","rb");
            // busca la segunda mitad del swap
            fseek(swapFile, indice * PAGESIZE, 0);
            do {
                struct PROCESSPAGETABLE leido;
                fread(&leido, sizeof(struct PROCESSPAGETABLE), 1 ,swapFile);
                // si el leido tiene datos, salir del while
                printf("HOLA CUCU %d \n",leido.framenumber);
                fflush(stdout);
                indice++;
            } while(indice < 16);

            fclose(swapFile);
        }


        // buscas en swap si existe la pagina nueva
        //      si si, te regresa su indice
        //      si no, lo escribe y te regresa el indice en donde lo escribio

        // en swap se intercambia el índice obtenido por el índice de la página vieja

        // la pagina vieja.presente = 0
        // la pagina vieja.framenumber = indice swapeado

        processpagetable[paginaNueva].presente = 1;
        processpagetable[paginaNueva].framenumber = marcoNuevo;
    }
    else {
        processpagetable[paginaNueva].presente = 1;
        processpagetable[paginaNueva].framenumber = marcoNuevo;

        swapFile = fopen("swap","wb");
        // busca la posicion de la página nueva en swap
        fseek(swapFile, processpagetable[paginaNueva].framenumber * PAGESIZE,0);
        // escribe el valor actual en swap
        fwrite(systemframetable[processpagetable[paginaNueva].framenumber].paddress,1,PAGESIZE,swapFile);

        fclose(swapFile);
    }

    return(1); // Regresar todo bien
}


int getfreeframe()
{
    int i;
    // Busca un marco libre en el sistema
    for(i=framesbegin;i<systemframetablesize+framesbegin;i++)
        if(!systemframetable[i].assigned)
        {
            systemframetable[i].assigned=1;
            break;
        }
    if(i<systemframetablesize+framesbegin)
        systemframetable[i].assigned=1;
    else
        i=-1;
    return(i);
}

int getOldestUsed()
{
    int i;
    int oldestIndex=0;
    // busca la pagina mas vieja que este asignada del proceso
    for(i = 0; i < PROCESSPAGETABLESIZE; i++)
    {
        if(systemframetable[processpagetable[i].framenumber].assigned && 
            processpagetable[i].tlastaccess < processpagetable[oldestIndex].tlastaccess)
        {
            oldestIndex=i;
        }
    }
    
    return oldestIndex;
}