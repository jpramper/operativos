#include <stdio.h>
#include "mmu.h"

#define RESIDENTSETSIZE 24

extern char *base;
extern int framesbegin;
extern int idproc;
extern int nframes;
extern int pagesperproc;

extern struct FRAMETABLE *frametable;
extern struct PAGETABLE pagetable[];

int pagefault(char *vaddress);
int getfreeframe();
int getLRU();
void execSwap(int newPageIndex);
int getFreeSwapAddress();
int hasFisicalAddress(int pageIndex);

/**
* @method pagefault Rutina de fallos de página
* @return failure/success
*/
int pagefault(char *vaddress)
{
  int pageIndex     = (int) vaddress >> 12; // page index in pagetable

  if(countframesassigned()< FRAMESPERPROC)
      pagetable[pageIndex].framenumber = getfreeframe();

  if(!hasFisicalAddress(pageIndex))
      execSwap(pageIndex);

  pagetable[pageIndex].presente    = TRUE;
  return SUCCESS;
}

/**
* @method getfreeframe
* @return index of new assigned frame
*/
int getfreeframe()
{
    int frame;
    for( frame=framesbegin ;frame<framesbegin+nframes ; frame++)
      if(!frametable[frame].assigned)
      {
        frametable[frame].assigned = TRUE;
        return frame;
      }

    return NINGUNO;
}

/**
* @method execSwap
* @return index of new assigned frame
*/
void execSwap(int newPageIndex)
{
    FILE   *swapFile = fopen("swap","r+b");

    // 1 nos aseguramos de tener los indices basicos:
    /*
    * newPageIndex --indice en pagetable de la pagina nueva
    * oldPageIndex --indice en pagetable de la pagina a desplazar
    * addressUp    --direccion fisica cuyo contendio cambiara de pagina vieja <-a nueva
    * addressDown  --direccion fisica cuyo contendio cambiara de pagina nueva <-a vieja
    */
    int oldPageIndex   = getLRU();//indice de la pagina removida

    if (pagetable[newPageIndex].framenumber == NINGUNO)
        pagetable[newPageIndex].framenumber = getFreeSwapAddress();

    int addressUp= pagetable[oldPageIndex].framenumber;
    int addressDown= pagetable[newPageIndex].framenumber;

    // 2 leemos contenidos de swap
    struct FRAMETABLE swapContentUp;
    struct FRAMETABLE swapContentDown;

    fseek(swapFile, addressUp-framesbegin,SEEK_SET);
    fread(&swapContentUp, BUFFERSIZE, 1 ,swapFile);
    fseek(swapFile, addressDown-framesbegin,SEEK_SET);
    fread(&swapContentDown, BUFFERSIZE, 1 ,swapFile);


    // 3 guardamos contenido bajo
    fseek(swapFile, addressDown-framesbegin,SEEK_SET);
    if(pagetable[oldPageIndex].modificado)
          fwrite(&frametable[addressUp], sizeof(struct FRAMETABLE), 1, swapFile);//si fue modificado, se guarda el que esta en ram
    else
        fwrite(&swapContentUp, sizeof(struct FRAMETABLE), 1, swapFile);//si no, se guarda el que esta en archivo

    pagetable[oldPageIndex].presente    = FALSE;
    pagetable[oldPageIndex].modificado  = FALSE;

    // 4 guardamos contenido alto
    if(swapContentDown.assigned)
        frametable[addressUp]= swapContentDown;

    // 5 actualizamos indices
    pagetable[newPageIndex].framenumber=addressUp;
    pagetable[oldPageIndex].framenumber=addressDown;

    fclose(swapFile);
}

/**
* @method getFreeSwapAddress
* @return address of free space
*/
int getFreeSwapAddress()
{
    FILE   *swapFile = fopen("swap","rb");
    struct  FRAMETABLE swapBuffer;
    int     freeAddress;

    fseek(swapFile, MEMSIZE,0);//se coloca en la parte baja de la memoria (frame 9-16)
    for( freeAddress=MEMSIZE ; freeAddress<SWAPSIZE ; freeAddress+=FRAMESIZE)
    {
        fread(&swapBuffer, BUFFERSIZE, 1 ,swapFile);
        if(swapBuffer.assigned<=0)
              break;
    }
    fclose(swapFile);

    if(freeAddress > SWAPSIZE)
      return ERROR;
    else
      return freeAddress+framesbegin;
}

/**
* @method getLRU
* @return Last Recently Used index from page table
*/
int getLRU()
{
    int i=0;
    int index=0;

    for(i = 0; i < PAGESPERPROC; i++)
      if(pagetable[i].tlastaccess < pagetable[index].tlastaccess &&
          hasFisicalAddress(i))
            index=i;

    return index;
}

int hasFisicalAddress(int pageIndex)
{
  return pagetable[pageIndex].framenumber >= framesbegin
      && pagetable[pageIndex].framenumber < MEMSIZE+framesbegin;
}
