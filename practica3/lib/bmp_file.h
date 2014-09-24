/**
*@method loadBMP
*@param filename  {char*}     image file input
*@param image     {IMAGE*}    image struct output
*@module file
*/
int loadBMP(char *filename, IMAGE *image)
{
  FILE *fin;
  int i=0;
  int totpixs=0;
  fin = fopen(filename, "rb+");

  if (fin == NULL)
    return(-1);

  // Leer encabezado
  fread(&image->header, sizeof(HEADER), 1, fin);

  // Probar si es un archivo BMP
  if (!((image->header.magic1 == 'B') && (image->header.magic2 == 'M')))
    return(-1);

  fread(&image->infoheader, sizeof(INFOHEADER), 1, fin);

  // Probar si es un archivo BMP 24 bits no compactado
  if (!((image->infoheader.bitsPerPixel == 24) && !image->infoheader.compression))
    return(-1);

  image->pixel=(PIXEL *)malloc(sizeof(PIXEL)*image->infoheader.cols*image->infoheader.rows);

  totpixs=image->infoheader.rows*image->infoheader.cols;

  while(i<totpixs)
  {
    fread(image->pixel+i, sizeof(PIXEL),512, fin);
    i+=512;
  }

  fclose(fin);

  return 0;
}

/**
*@method saveBMP
*@param filename  {char*}     image file output
*@param image     {IMAGE*}    image struct input
*@module file
*/
int saveBMP(char *filename, IMAGE *image)
{
  FILE *fout;
  int i,totpixs;

  fout=fopen(filename,"wb");
  if (fout == NULL)
    return(-1);// Error

  // Escribe encabezado
  fwrite(&image->header, sizeof(HEADER), 1, fout);

  // Escribe información del encabezado
  fwrite(&image->infoheader, sizeof(INFOHEADER), 1, fout);

  i=0;
  totpixs=image->infoheader.rows*image->infoheader.cols;
  while(i<totpixs)
  {
    fwrite(image->pixel+i,sizeof(PIXEL),512,fout);
    i+=512;
  }

  fclose(fout);

  return 0;
}
