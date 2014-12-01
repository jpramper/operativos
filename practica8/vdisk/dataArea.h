#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "logicSectors.h"

extern short secboot_en_memoria;  //bandera de sec boot
extern struct SECBOOT secBoot;	   // estructura secboot

extern short inodesmap_en_memoria; // inode map flag
extern unsigned char iNodesMap[SECSIZE]; //mapa de bits de iNode

extern short datamap_en_memoria;	//bandera de el mapa de datos
extern unsigned char dataMap[SECSIZE]; // el mapa de datos

// ******************************************************************************
// Para el mapa de bits del área de de datos
// ******************************************************************************

int nextfreeblock()
{
	int i,j;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!datamap_en_memoria)
	{
		if (vdreadls(dataMapLs(),1, &dataMap) == -1) //inicializamos sector boot si no existia antes
			return -1;
		datamap_en_memoria=1;
	}

	i=0;
	while(dataMap[i]==0xFF && i<secBoot.sec_mapa_bits_bloques*SECSIZE)
		i++;

	if(i<secBoot.sec_mapa_bits_bloques*SECSIZE)
	{
		j=0;
		while(dataMap[i] & (1<<j) && j<8)
			j++;

		return(i*8+j);
	}
	else
		return(-1);

		
}

int isblockfree(int block)
{
	int offset=block/8;
	int shift=block%8;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!datamap_en_memoria)
	{
		if (vdreadls(dataMapLs(),1, &dataMap) == -1) //inicializamos sector boot si no existia antes
			return -1;
		datamap_en_memoria=1;
	}

	if(dataMap[offset] & (1<<shift))
		return(0);

	return(1);
}

int assignblock(int block)
{
	int offset=block/8;
	int shift=block%8;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!datamap_en_memoria)
	{
		if (vdreadls(dataMapLs(),1, &dataMap) == -1) //inicializamos sector boot si no existia antes
			return -1;
		datamap_en_memoria=1;
	}

	dataMap[offset]|=(1<<shift);

	if (vdwritels(dataMapLs(),1,(char *) &dataMap) == -1)
		return -1;
	//for(i=0;i<secboot.sec_mapa_bits_bloques;i++)
	//	vdwriteseclog(mapa_bits_bloques+i,blocksmap+i*512);
	return(1);
}

int unassignblock(int block)
{
	int offset=block/8;
	int shift=block%8;

	char mask;
	int sector;
	int i;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!datamap_en_memoria)
	{
		if (vdreadls(dataMapLs(),1, &dataMap) == -1) //inicializamos sector boot si no existia antes
			return -1;
		datamap_en_memoria=1;
	}

	dataMap[offset]&=(char) ~(1<<shift);

	if (vdwritels(dataMapLs(),1,(char *) &dataMap) == -1)
		return -1;

	return(1);
}

//*******************************************************************************
// Lectura y escritura de bloques
// ******************************************************************************

int writeblock(int block,char *buffer)
{
	int i;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	// direccion logica , 1, buffer
	// si escribiera un sector (block * SECSIZE) + dataBlockLs()

	vdwritels(dataBlockLs()+(block*secBoot.sec_x_bloque* SECSIZE), secBoot.sec_x_bloque, buffer);
	//vdwritels(dataBlockLs()+((block-1)*secBoot.sec_x_bloque* SECSIZE), secBoot.sec_x_bloque, buffer); // a ver cual es

	return(1);	
}

int readblock(int block,char *buffer)
{
	int i;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	vdreadls(dataBlockLs()+(block*secBoot.sec_x_bloque* SECSIZE), secBoot.sec_x_bloque, buffer);
	return(1);	
}