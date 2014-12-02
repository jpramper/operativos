#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "logicSectors.h"

extern short secboot_en_memoria;  // sec boot
extern struct SECBOOT secBoot;
extern short inodesmap_en_memoria; // inode
extern unsigned char iNodesMap[SECSIZE];
extern short datamap_en_memoria;	//mapa de datos
extern unsigned char dataMap[SECSIZE];

//////////////////////////////////////////////////////////
/*Method Declaration Area*/
// Para el mapa de bits del área de de datos
int nextfreeblock();
int isblockfree(int block);
int assignblock(int block);
int unassignblock(int block);
// Lectura y escritura de bloques
int writeblock(int block,char *buffer);
int readblock(int block,char *buffer);
/**/
//////////////////////////////////////////////////////////

// ******************************************************************************
// Para el mapa de bits del área de de datos
// ******************************************************************************

int nextfreeblock()
{
	int i,j;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_datamap()==ERROR) return ERROR;

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

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_datamap()==ERROR) return ERROR;

	if(dataMap[offset] & (1<<shift))
		return(0);

	return(1);
}

int assignblock(int block)
{
	int offset=block/8;
	int shift=block%8;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_datamap()==ERROR) return ERROR;

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

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_datamap()==ERROR) return ERROR;

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

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_datamap()==ERROR) return ERROR;

	// direccion logica , 1, buffer
	// si escribiera un sector (block * SECSIZE) + dataBlockLs()
	vdwritels(dataBlockLs()+(block*secBoot.sec_x_bloque), secBoot.sec_x_bloque, buffer);

	return(1);
}

int readblock(int block,char *buffer)
{
	int i;

	if(check_secboot()==ERROR) return ERROR;

	vdreadls(dataBlockLs()+(block*secBoot.sec_x_bloque), secBoot.sec_x_bloque, buffer);
	return(1);
}
