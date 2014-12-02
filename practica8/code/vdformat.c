#include <stdio.h>
#include "vdisk.h"

int main(int argc,char *argv[])
{
	int i;
	unsigned char buffer[SECSIZE];

	char nombrefile[15] = "disco";

	strcat(nombrefile, argv[1]);
	strcat(nombrefile, ".vd");

	printf("argv %s\n", nombrefile);

	if(argc <= 1)
	{
		printf("No hay argumentos, meteme algo porfa \n");
		return (0);
	}

	// asignar valores de sector 0 (boot)
	struct SECBOOT mbr;  // master boot record

	mbr.jump[0] = 0;
	mbr.jump[1] = 0;
	mbr.jump[2] = 0;
	mbr.jump[3] = 0;

	for (i = 1; i < 8; i++)
		mbr.nombre_disco[i] = 0;
	strcat(mbr.nombre_disco, argv[1]);

	mbr.sec_res = 0;
	mbr.sec_mapa_bits_nodo_i = 1;
	mbr.sec_mapa_bits_bloques = 2;
	mbr.sec_tabla_nodos_i = 3;
	mbr.sec_log_unidad= 11;
	mbr.sec_x_bloque= SECxBLOCK;
	mbr.heads = HEADS;
	mbr.cyls = CYLINDERS;
	mbr.secfis = SECTORS;

	// limpiamos basura del mbr
	for (i = 0; i < 487; i++)
		mbr.restante[i] = 0;


	// escribimos el master boot record
	vdwritesector(atoi(argv[1]),0,0,1,1,&mbr);

	// limpiamos el buffer
	for (i = 0; i < SECSIZE; i++)
		buffer[i] = 0;

	// escribimos la segunda seccion vacia
	vdwritesector(atoi(argv[1]),0,0,2,1,buffer);

	//asignar valor 1 del sector 3 (mapa datos)
	buffer[0] = 1;

	//escritura de la tercera seccion
	vdwritesector(atoi(argv[1]),0,0,3,1,buffer);

	return (0);
}
