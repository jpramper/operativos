#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define DIF 16

// NOMBRE DEL ARCHIVO A PROCESAR
char filename[]="/home/john/practica3/img/rocks";

#pragma pack(2) // Empaquetado de 2 bytes
typedef struct {
  // firma
  unsigned char magic1; // 'B'
  unsigned char magic2; // 'M'

  unsigned int size; // Tamaño
  unsigned short int reserved1, reserved2; // campo reservado
  unsigned int pixelOffset; // offset a la imagen
} HEADER;

#pragma pack() // Empaquetamiento por default
typedef struct {
  unsigned int size;// Tamaño de este encabezado INFOHEADER
  int cols, rows;// Renglones y columnas de la imagen
  unsigned short int planes; // planos (siempre 1)
  unsigned short int bitsPerPixel; // profundidad (Bits por pixel)
  unsigned int compression; // tipo de compresión
  unsigned int cmpSize; // tamaño en bytes de la imágen
  int xScale, yScale; // resolución horizontal y vertical
  unsigned int numColors; // número de colores de la paleta
  unsigned int importantColors; // número de colores importantes de la paleta (0 es todos)
} INFOHEADER;

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} PIXEL;

typedef struct {
  HEADER header; // encabezado de archivo
  INFOHEADER infoheader; // encabezado de información
  PIXEL *pixel; // cuerpo de datos
} IMAGE;

IMAGE imagenfte,imagendst;

int loadBMP(char *filename, IMAGE *image)
{
  FILE *fin; // apuntador al flujo del archivo

  int i = 0; // contador de lectura
  int totpixs = 0; // límite para el contador de lectura
  
  // abrir el flujo al archivo como binary read/update
  fin = fopen(filename, "rb+");

  // Si el archivo no existe
  if (fin == NULL)
    return(-1);

  // Leer encabezado de archivo
  fread(&image->header, sizeof(HEADER), 1, fin);

  // Probar si es un archivo BMP
  if (!((image->header.magic1 == 'B') && (image->header.magic2 == 'M')))
    return(-1);

  // leer encabezado de información
  fread(&image->infoheader, sizeof(INFOHEADER), 1, fin);

  // Probar si es un archivo BMP 24 bits no compactado
  if (!((image->infoheader.bitsPerPixel == 24) && !image->infoheader.compression))
    return(-1);

  // reservar memoria para el cuerpo de la imágen
  // (tamaño de pixel) x columnas x filas 
  image->pixel = (PIXEL *) malloc(sizeof(PIXEL)*image->infoheader.cols*image->infoheader.rows);

  // definir el número de pixeles a leer: filas x columnas
  totpixs = image->infoheader.rows*image->infoheader.cols;

  // mientras no se haya alcanzado el total de pixeles
  while(i < totpixs)
  {
    // leer un bloque de 512 pixeles
    fread(image->pixel+i, sizeof(PIXEL), 512, fin);
    // aumentar el contador de lecturas
    i += 512;
  }

  fclose(fin);
}


int saveBMP(char *filename, IMAGE *image)
{
  // apuntador al flujo de salida
  FILE *fout;

  int i = 0; // contador de lectura
  int totpixs = 0; // límite para el contador de lectura

  // abrir el flujo al archivo como binary write
  fout = fopen(filename,"wb");

  // verificar creación del flujo
  if (fout == NULL)
    return(-1);// Error

  // Escribe encabezado de archivo
  fwrite(&image->header, sizeof(HEADER), 1, fout);

  // Escribe encabezado de información
  fwrite(&image->infoheader, sizeof(INFOHEADER), 1, fout);

  // definir el número de pixeles a escribir: filas x columnas
  totpixs = image->infoheader.rows*image->infoheader.cols;

  // mientras no se haya alcanzado el total de pixeles
  while(i < totpixs)
  {
    // escribir un bloque de 512 pixeles
    fwrite(image->pixel+i, sizeof(PIXEL), 512, fout);
    // aumentar el contador de lecturas
    i += 512;
  }

  fclose(fout);
}

unsigned char blackandwhite(PIXEL p)
{
  // reporta el producto de una porción de 1 para cada tono (0.3 + 0.59 + 0.11 = 1.0)
  return ((unsigned char) (0.3 * ((float)p.red) + 0.59 * ((float)p.green) + 0.11 * ((float)p.blue)));
}

void processBMP(IMAGE *imagefte, IMAGE *imagedst)
{
  int i,j; // indicadores de posición actual
  PIXEL *pfte,*pdst; // apuntadores para el pixel actual
  PIXEL *v0,*v1,*v2,*v3,*v4,*v5,*v6,*v7; // apuntadores para los pixeles vecinos
  int imageRows, imageCols; // límites para los contadores 

  // copia los encabezados de la imágen fuente a la imágen destino
  memcpy(imagedst, imagefte, sizeof(IMAGE)-sizeof(PIXEL *));
  // obten los límites para los contadores de filas y columnas
  imageRows = imagefte->infoheader.rows;
  imageCols = imagefte->infoheader.cols;

  // reserva la memoria en la imágen destino para los pixeles
  imagedst->pixel = (PIXEL *) malloc(sizeof(PIXEL)*imageRows*imageCols);

  short p;
  // para cada fila
  for(i = 1; i < imageRows-1; i++)
    // para cada columna
    for(j = 1; j < imageCols-1; j++)
      {
        // encuentra al pixel actual
        pfte = imagefte->pixel+imageCols*i+j;

        // encuentra el pixel destino
        pdst=imagedst->pixel+imageCols*i+j;

        p = blackandwhite(*pfte);

        // encuentra a los vecinos
        v0=pfte-imageCols-1;  // renglon arriba a la izquierda
        v1=pfte-imageCols;    // renglon arriba
        v2=pfte-imageCols+1;  // renglon arriba a la derecha
        v3=pfte-1;            // mismo renglon a la izquierda
        v4=pfte+1;            // mismo renglon a la derecha
        v5=pfte+imageCols-1;  // renglon abajo a la izquierda
        v6=pfte+imageCols;    // renglon abajo
        v7=pfte+imageCols+1;  // renglon abajo a la derecha

        // si hay un cambio de tono mayor al DIF entre el pixel fuente y algun vecino
        if(abs(p-blackandwhite(*v0))>DIF ||
        abs(p-blackandwhite(*v1))>DIF ||
        abs(p-blackandwhite(*v2))>DIF ||
        abs(p-blackandwhite(*v3))>DIF ||
        abs(p-blackandwhite(*v4))>DIF ||
        abs(p-blackandwhite(*v5))>DIF ||
        abs(p-blackandwhite(*v6))>DIF ||
        abs(p-blackandwhite(*v7))>DIF)
        {
          // pinta el pixel de negro
          pdst->red=0;
          pdst->green=0;
          pdst->blue=0;
        }
        else // si no hay cambio
        {
          // pinta el pixel de blanco
          pdst->red=255;
          pdst->green=255;
          pdst->blue=255;
        }
      }
}

int main()
{
  // variables para manejo de tiempo
  struct timeval start_ts;
  struct timeval stop_ts;
  struct timeval elapsed_time;

  int res; // variable de resultado de procesos
  
  char namedest[100]; // variable para el nombre del destino

  // obtener el tiempo inicial
  gettimeofday(&start_ts, NULL);

  // generar los nombres de los archivos fuente y destino
  strcpy(namedest, strtok(filename,"."));
  strcat(filename,".bmp");
  strcat(namedest,"_secuencial_cache.bmp");

  printf("Archivo fuente %s\n",filename);
  printf("Archivo destino %s\n",namedest);

  // cargar el archivo en memoria
  res = loadBMP(filename, &imagenfte);
  // verificar el cargado
  if(res == -1)
  {
    fprintf(stderr, "Error al abrir imagen\n");
    exit(1);
  }

  printf("Procesando imagen de: Renglones = %d, Columnas =%d\n", imagenfte.infoheader.rows, imagenfte.infoheader.cols);
  // ejecutar el procesamiento
  processBMP(&imagenfte, &imagendst);

  // 
  res = saveBMP(namedest, &imagendst);
  // verificar la escritura
  if(res == -1)
  {
    fprintf(stderr, "Error al escribir imagen\n");
    exit(1);
  }

  // obtener el tiempo final
  gettimeofday(&stop_ts, NULL);

  // calcular e imprimir tiempo
  timersub(&stop_ts, &start_ts, &elapsed_time);

  printf("------------------------------\n");
  printf("TIEMPO TOTAL, %ld.%ld segundos\n",elapsed_time.tv_sec, elapsed_time.tv_usec);
}
