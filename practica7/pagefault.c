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
int getFrameIndex(int pageIndex);
int getFrameAddress(int pageIndex);

/**
* @method pagefault Rutina de fallos de página
* @return failure/success
*/
int pagefault(char *vaddress)
{
  FILE *swapFile;
  int page     = (int) vaddress >> 12;
  int frame    = NINGUNO;

  if(countframesassigned()< 2)
      frame = getfreeframe();

  if(frame == NINGUNO)
  {
      // algoritmo de reemplazo
      int oldPage= getLRU();
      printf("HOLAAAAAA SOY %d,%d Y ME VAN A REEMPLAZAR POR LA PAGINA %d MODIFED=%d \n",idproc,page,oldPage,pagetable[oldPage].modificado);
      fflush(stdout);

      // si la pagina de salida ha sido modificada, se guarda
      if (pagetable[oldPage].modificado)
      {
          printf("TOY SUCIO :B\n");
          fflush(stdout);

          pagetable[oldPage].modificado= FALSE;

          swapFile = fopen("swap","wb");
          fseek(swapFile, getFrameAddress(oldPage),0);
          fwrite(&pagetable[oldPage], PAGESIZE, 1, swapFile);

          fclose(swapFile);
      }

      if (pagetable[page].framenumber == NINGUNO)
      {

      }

      return -1;
  }

  printf("HOLAAAAAA MI ESTAN GUARDANDO %d,%d EN %d\n",idproc,page,frame);
  fflush(stdout);

  pagetable[page].presente    = TRUE;
  pagetable[page].modificado  = TRUE;
  pagetable[page].framenumber = frame;

  return SUCCESS;
}

/**
* @method getfreeframe
* @return index of new assigned frame
*/
int getfreeframe()
{
    int frame;
    for( frame=framesbegin ;frame<nframes+framesbegin ; frame++)
      if(!frametable[frame].assigned)
      {
        frametable[frame].assigned = TRUE;
        return frame;
      }

    return NINGUNO;
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
          getFrameIndex(i)!=NINGUNO)
        {
            index=i;
        }

    return index;
}

/**
* @method getFrameIndex
* @return index position of a frame mapped in a pagetable
*/
int getFrameIndex(int pageIndex)
{
  return pagetable[pageIndex].framenumber;
}

/**
* @method getFrameAddress
* @return position of a page mapped to a frame inside disk
*/
int getFrameAddress(int pageIndex)
{
  return (pagetable[pageIndex].framenumber-framesbegin) * FRAMESIZE;
}
