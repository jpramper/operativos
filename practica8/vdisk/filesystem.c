
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "vdisk.c" 

short secboot_en_memoria;
struct SECBOOT secboot;

/**
* donde inicia el mapa de bits
* @method mbrLs master boot logic sector
*/
int mbrLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}

    return 0;//TODO on ta ??
}

/**
* donde inicia el mapa de bits del area de datos
* @method dataLs data logic sector
*/
int dataLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}

    return secboot.sec_mapa_bits_bloques;
}

/**
* donde inicia el mapa de bits area de nodos i
* @method inodeLs i-nodes logic sector
*/
int inodeLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}

    return secboot.sec_tabla_nodos_i;
}

/**
* donde inicia el area de archivos
* @method fileLs 
*/
int fileLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}

    return 0;//TODO on ta ??
}