//compile with gcc -pthread -o borders_pthread borders_pthread.c
//run with ./borders_phread
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "lib/bmp_structs.h"
#include "lib/bmp_file.h"
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <linux/sched.h>

/**
*@config DIF When a pixel P>=DIF, pixel is set to black.
*/
#define DIF 16
#define NUM_THREADS 4
#define FIBER_STACK 1024*64

char filename[]="/home/cury/ITESO/Operativos/practica3/img/rocks.bmp";

unsigned char blackandwhite(PIXEL p)
{
  return((unsigned char) (0.3*((float)p.red)+0.59*((float)p.green)+0.11*((float)p.blue)));
}

void processBMP(IMAGE *imagefte, IMAGE *imagedst);
void *threadProcessBMP(void *threadid);

int main()
{
  // variables para manejo de tiempo
  struct timeval start_ts;
  struct timeval stop_ts;
  struct timeval elapsed_time;

  IMAGE imagefte;//Imagen fuente
  IMAGE imagedst;//Imagen destino

  //file name
  char namedest[80];
  strcpy(namedest,strtok(filename,"."));
  strcat(filename,".bmp");
  strcat(namedest,"_P.bmp");
  printf("Source file: %s\n",filename);
  printf("Target file: %s\n",namedest);

  // obtener el tiempo inicial
  gettimeofday(&start_ts, NULL);

  //load
  if(loadBMP(filename,&imagefte)==-1)
  {
    fprintf(stderr,"ERROR: Couldn't open image\n");
    exit(1);
  }
  printf("\nProcessing image: ROWS= %d, COLUMNS= %d\n\n",imagefte.infoheader.rows,imagefte.infoheader.cols);

  //process
  processBMP(&imagefte,&imagedst);

  //save
  puts("guardar");
  if(saveBMP(namedest,&imagedst)==-1)
  {
    fprintf(stderr,"Error al escribir imagen\n");
    exit(1);
  }

  // obtener el tiempo final
  gettimeofday(&stop_ts, NULL);

  // calcular e imprimir tiempo
  timersub(&stop_ts, &start_ts, &elapsed_time);

  printf("------------------------------\n");
  printf("TIEMPO TOTAL, %ld.%ld segundos\n",elapsed_time.tv_sec, elapsed_time.tv_usec);
}

///////////////////////////////////////////////////
//THREAD IMPLEMENTATION

/**
*@class thread_info
*@module structures
*/
struct thread_info {
   int nThread;
   int fromRow;
   int status;
   int toRow;
   int imageCols;
   IMAGE *imagefte;//Imagen fuente
   IMAGE *imagedst;//Imagen destino
};

/**
*main thread
*@method processBMP
*/
void processBMP(IMAGE *imagefte, IMAGE *imagedst)
{
  memcpy(imagedst,imagefte,sizeof(IMAGE)-sizeof(PIXEL *));
  int imageRows = imagefte->infoheader.rows;
  int imageCols = imagefte->infoheader.cols;
  imagedst->pixel=(PIXEL *)malloc(sizeof(PIXEL)*imageRows*imageCols);

  void* stack;
  pid_t pid[NUM_THREADS];
  struct thread_info tinfo[NUM_THREADS];
  int status;
  int i;

  for(i=0; i<NUM_THREADS; i++)
  {
    /*
    * assign rows to thread
    */
    tinfo[i].nThread = i;
    tinfo[i].status = 1;
    if (i==0)
	tinfo[i].fromRow = 1;
    else
	tinfo[i].fromRow = i*(imageRows/NUM_THREADS);
    if((i+1)>=NUM_THREADS)
      tinfo[i].toRow = imageRows-1;
    else
      tinfo[i].toRow = ((i+1)*(imageRows/NUM_THREADS))-1;
    tinfo[i].imageCols = imageCols;
    tinfo[i].imagefte = imagefte;
    tinfo[i].imagedst = imagedst;

    /*
    * Clones
    */

    // Allocate the stack
    stack = malloc(FIBER_STACK);
    if ( stack == 0 )
    {
        perror( "malloc: could not allocate stack" );
        exit( 1 );
    }


    //create the clone
    pid[i] = clone( &threadProcessBMP, (char*) stack + FIBER_STACK, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_IO | CLONE_VM, &tinfo[i]);
    if ( pid[i] == -1 )
      {
        exit( 2 );
      }	
  }

  for(i=0; i<NUM_THREADS; i++)
  {	
    while(tinfo[i].status){
    }
  }
  free( stack );
}

void *threadProcessBMP(void *arg)
{
    struct thread_info *tinfo = arg;
    printf("Thread %d working from %d to %d Row\n",tinfo->nThread,tinfo->fromRow,tinfo->toRow);

    int imageCols= tinfo->imageCols;
    IMAGE *imagefte= tinfo->imagefte;
    IMAGE *imagedst= tinfo->imagedst;
    PIXEL *pfte,*pdst;
    PIXEL *v0,*v1,*v2,*v3,*v4,*v5,*v6,*v7;

    int i,j;
    short p;
    for( i=tinfo->fromRow ; i<=tinfo->toRow ; i++)
      for(j=1;j< imageCols-1;j++)
        {
          pfte=imagefte->pixel+imageCols*i+j;
          v0=pfte-imageCols-1;
          v1=pfte-imageCols;
          v2=pfte-imageCols+1;
          v3=pfte-1;
          v4=pfte+1;
          v5=pfte+imageCols-1;
          v6=pfte+imageCols;
          v7=pfte+imageCols+1;

          p = blackandwhite(*pfte);

          pdst=imagedst->pixel+imageCols*i+j;

          if(abs(p-blackandwhite(*v0))>DIF ||
          abs(p-blackandwhite(*v1))>DIF ||
          abs(p-blackandwhite(*v2))>DIF ||
          abs(p-blackandwhite(*v3))>DIF ||
          abs(p-blackandwhite(*v4))>DIF ||
          abs(p-blackandwhite(*v5))>DIF ||
          abs(p-blackandwhite(*v6))>DIF ||
          abs(p-blackandwhite(*v7))>DIF)
          {
            pdst->red=0;
            pdst->green=0;
            pdst->blue=0;
          }
          else
          {
            pdst->red=255;
            pdst->green=255;
            pdst->blue=255;
          }
        }
	
	tinfo->status = 0;
        printf("Thread %d finished\n",tinfo->nThread);
}
