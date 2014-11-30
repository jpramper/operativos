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

int nextfreeblock()
{
	int i,j;
	int result;

	if(!secboot_en_memoria)
	{
		result=vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

	mapa_bits_bloques = secboot.sec_res+secboot.sec_mapa_bits_nodos_i;

	if(!blocksmap_en_memoria)
	{
		for(i=0;i<secboot.sec_mapa_bits_bloques;i++)
			result=vdreadseclog(mapa_bits_bloques+i,blocksmap+i*512);
		blocksmap_en_memoria=1;
	} 

	i=0;
	while(blocksmap[i]==0xFF && i<secboot.sec_mapa_bits_bloques*512)
		i++;

	if(i<secboot.sec_mapa_bits_bloques*512)
	{
		j=0;
		while(blocksmap[i] & (1<<j) && j<8)
			j++;

		return(i*8+j);
	}
	else
		return(-1);

		
}

int assignblock(int block)
{
	int offset=block/8;
	int shift=block%8;
	int result;
	int i;
	int sector;

	if(!secboot_en_memoria)
	{
		result=vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

	mapa_bits_bloques= secboot.sec_res+secboot.sec_mapa_bits_nodos_i;

	if(!blocksmap_en_memoria)
	{
		for(i=0;i<secboot.sec_mapa_bits_bloques;i++)
			result=vdreadseclog(mapa_bits_bloques+i,blocksmap+i*512);
		blocksmap_en_memoria=1;
	} 

	blocksmap[offset]|=(1<<shift);

	sector=offset/512;
	vdwriteseclog(mapa_bits_bloques+sector,blocksmap+sector*512);
	//for(i=0;i<secboot.sec_mapa_bits_bloques;i++)
	//	vdwriteseclog(mapa_bits_bloques+i,blocksmap+i*512);
	return(1);
}

int unassignblock(int block)
{
	int offset=block/8;
	int shift=block%8;
	int result;
	char mask;
	int sector;
	int i;

	if(!secboot_en_memoria)
	{
		result=vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

	mapa_bits_bloques= secboot.sec_res+secboot.sec_mapa_bits_nodos_i;

	if(!blocksmap_en_memoria)
	{
		for(i=0;i<secboot.sec_mapa_bits_bloques;i++)
		 	result=vdreadseclog(mapa_bits_bloques+i,blocksmap+i*512);
		blocksmap_en_memoria=1;
	}

	blocksmap[offset]&=(char) ~(1<<shift);

	sector=offset/512;
	vdwriteseclog(mapa_bits_bloques+sector,blocksmap+sector*512);
	// for(i=0;i<secboot.sec_mapa_bits_bloques;i++)
	//	vdwriteseclog(mapa_bits_bloques+i,blocksmap+i*512);
	return(1);
}




//*******************************************************************************
// Lectura y escritura de bloques
// ******************************************************************************

int writeblock(int block,char *buffer)
{
	int result;
	int i;
	if(!secboot_en_memoria)
	{
		result=vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

	inicio_area_datos=secboot.sec_res+secboot.sec_mapa_bits_nodos_i +secboot.sec_mapa_bits_bloques+secboot.sec_tabla_nodos_i;


	for(i=0;i<secboot.sec_x_bloque;i++)
		vdwriteseclog(inicio_area_datos+(block-1)*secboot.sec_x_bloque+i,buffer+512*i);
	return(1);	
}

int readblock(int block,char *buffer)
{
	int result;
	int i;

	if(!secboot_en_memoria)
	{
		result=vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}
	inicio_area_datos=secboot.sec_res+secboot.sec_mapa_bits_nodos_i+secboot.sec_mapa_bits_bloques+secboot.sec_tabla_nodos_i;

	for(i=0;i<secboot.sec_x_bloque;i++)
		vdreadseclog(inicio_area_datos+(block-1)*secboot.sec_x_bloque+i,buffer+512*i);
	return(1);	
}