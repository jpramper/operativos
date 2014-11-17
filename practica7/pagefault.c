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
int getSwapAddress(int pageIndex);

/**
* @method pagefault Rutina de fallos de página
* @return failure/success
*/
int pagefault(char *vaddress)
{
  FILE    *swapFile;
  struct  FRAMETABLE swapBuffer;

  int page     = (int) vaddress >> 12;
  int frame    = NINGUNO;


  if(countframesassigned()< FRAMESPERPROC)
      frame = getfreeframe();

  // algoritmo de reemplazo
  if(frame == NINGUNO)
  {
      int                 oldPage= getLRU();//indice de la pagina desplazada
      int                 frameUp   = getFrameIndex(oldPage);
      int                 frameDown = getFrameIndex(page);

      struct  FRAMETABLE  oldFrameHandler= frametable[frameUp]; //frame desplazado
      frametable[frameUp].assigned= FALSE;// resetea el frame

      // si la pagina nueva no tiene un espacio en swap asignado, se le busca uno libre
      if (frameDown == NINGUNO)
      {
          swapFile = fopen("swap","rb");

          for( frameDown=MEMSIZE ; frameDown<SWAPSIZE ; frameDown+=FRAMESIZE)
          {
              fread(&swapBuffer, BUFFERSIZE, 1 ,swapFile);
              if(swapBuffer.assigned<=0)
                    break;
          }
          fclose(swapFile);

          if(frameDown > SWAPSIZE)
            return ERROR;
          else
            frameDown+=framesbegin;
      }
      // si ya tiene su espacio en la parte baja, se copia a la parte alta
      else
      {
          frameDown= getFrameIndex(page);

          swapFile = fopen("swap","rb");
          fseek(swapFile, getSwapAddress(page),0);
          fread(&frametable[getFrameIndex(oldPage)], sizeof(struct FRAMETABLE), 1 ,swapFile);
          fclose(swapFile);
      }

      //escribe la parte baja (el frame desplazado)
      swapFile = fopen("swap","wb");
      fseek(swapFile, frameDown,0);
      fwrite(&oldFrameHandler, PAGESIZE, 1, swapFile);
      fclose(swapFile);

      pagetable[oldPage].presente= FALSE;
      pagetable[oldPage].modificado= FALSE;
      pagetable[oldPage].framenumber= frameDown;

      frame= frameUp;
  }

  fflush(stdout);

  //actualiza parte alta
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
    for( frame=framesbegin ;frame<framesbegin+nframes ; frame++)
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
* @method getSwapAddress
* @return position of a page mapped to a frame inside disk
*/
int getSwapAddress(int pageIndex)
{
  return (pagetable[pageIndex].framenumber-framesbegin) * FRAMESIZE;
}
