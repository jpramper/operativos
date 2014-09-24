/**
*2-Byte packaging
*@class HEADER
*@module structures
*/
#pragma pack(2)
typedef struct {
  unsigned char magic1; // 'B'
  unsigned char magic2; // 'M'
  unsigned int size; // Tamaño
  unsigned short int reserved1, reserved2;
  unsigned int pixelOffset; // offset a la imagen
} HEADER;

/**
*Default packaging
*@class INFOHEADER
*@module structures
*/
#pragma pack()
typedef struct {
  unsigned int size;// Tamaño de este encabezado INFOHEADER
  int cols, rows;// Renglones y columnas de la imagen
  unsigned short int planes;
  unsigned short int bitsPerPixel; // Bits por pixel
  unsigned int compression;
  unsigned int cmpSize;
  int xScale, yScale;
  unsigned int numColors;
  unsigned int importantColors;
} INFOHEADER;

/**
*@class PIXEL
*@module structures
*/
typedef struct {
unsigned char red;
unsigned char green;
unsigned char blue;
} PIXEL;

/**
*@class IMAGE
*@module structures
*/
typedef struct {
  HEADER header;
  INFOHEADER infoheader;
  PIXEL *pixel;
} IMAGE;
