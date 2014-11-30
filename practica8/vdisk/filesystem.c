
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "vdisk.c" 

short secboot_en_memoria;
struct SECBOOT secboot;
struct INODE inode;
short inodesmap_en_memoria;

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
* donde inicia el mapa de bits area de nodos i
* @method inodeMapLs i-nodes logic sector
*/
int inodeMapLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}

    return secboot.sec_tabla_nodos_i;
}

/**
* donde inicia el mapa de bits del area de datos
* @method dataMapLs data logic sector
*/
int dataMapLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}

    return secboot.sec_mapa_bits_bloques;
}



/**
* donde inicia el area de nodos i
* @method  iNodeLs
*/

int iNodeLs()
{
	if(!secboot_en_memoria)
	{
		vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}

    return secboot.sec_tabla_nodos_i;//TODO on ta ??
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

    return secboot.sec_log_unidad;//TODO on ta ??
}

// *************************************************************************
// Para el mapa de bits del área de nodos i
// *************************************************************************

int isinodefree(int inode)
{
	int offset=inode/8;
	int shift=inode%8;
	int result;

	// Checar si el sector del superbloque está en memoria
	if(!secboot_en_memoria)
	{
		// Si no está en memoria, cárgalo
		result=vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}
	mapa_bits_nodos_i= secboot.sec_res; 	//Usamos la información del superbloque para 
						//determinar en que sector inicia el 
						// mapa de bits de nodos i 
					
	// Ese mapa está en memoria
	if(!inodesmap_en_memoria)
	{
		// Si no está en memoria, hay que leerlo del disco
		result=vdreadseclog(mapa_bits_nodos_i,inodesmap);
		inodesmap_en_memoria=1;
	}


	if(inodesmap[offset] & (1<<shift))
		return(0);
	else
		return(1);
}	

int nextfreeinode()
{
	int i,j;
	int result;

	if(!secboot_en_memoria)
	{
		result=vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}
	mapa_bits_nodos_i= secboot.sec_res;

	if(!inodesmap_en_memoria)
	{
		result=vdreadseclog(mapa_bits_nodos_i,inodesmap);
		inodesmap_en_memoria=1;
	}

	// Recorrer byte por byte mientras sea 0xFF sigo recorriendo
	i=0;
	while(inodesmap[i]==0xFF && i<secboot.sec_mapa_bits_nodos_i*512)
		i++;

	if(i<secboot.sec_mapa_bits_nodos_i*512)
	{
		j=0;
		while(inodesmap[i] & (1<<j) && j<8)
			j++;

		return(i*8+j);
	}
	else
		return(-1);	
}