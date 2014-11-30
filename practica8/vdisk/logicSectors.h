#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "vdisk.h" 

extern short secboot_en_memoria;  //bandera de sec boot
extern struct SECBOOT secBoot;	   // estructura secboot

extern short inodesmap_en_memoria; // inode map flag
extern unsigned char iNodesMap[SECSIZE]; //mapa de bits de iNode

extern short datamap_en_memoria;	//bandera de el mapa de datos
extern unsigned char dataMap[SECSIZE]; // el mapa de datos

/**
* donde inicia el mapa de bits
* @method mbrLs master boot logic sector
*/
int mbrLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

    return 0; //secBoot.sec_res;
}

/**
* donde inicia el mapa de bits area de nodos i
* @method inodeMapLs i-nodes logic sector
*/
int inodeMapLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

    return secBoot.sec_mapa_bits_nodo_i;
}

/**
* donde inicia el mapa de bits del area de datos
* @method dataMapLs data logic sector
*/
int dataMapLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

    return secBoot.sec_mapa_bits_bloques;
}

/**
* donde inicia el area de nodos i
* @method  iNodeLs
*/
int iNodeLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

    return secBoot.sec_tabla_nodos_i;
}

/**
* donde inicia el area de archivos
* @method dataBlockLs 
*/
int dataBlockLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secBoot);
		secboot_en_memoria=1;
	}

    return secBoot.sec_log_unidad;
}