#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "vdisk.h"

extern short secboot_en_memoria;  // sec boot
extern struct SECBOOT secBoot;
extern short inodesmap_en_memoria; // inode
extern unsigned char iNodesMap[SECSIZE];
extern short datamap_en_memoria;	//mapa de datos
extern unsigned char dataMap[SECSIZE];

//////////////////////////////////////////////////////////
/*Method Declaration Area*/
//check sector (return ERROR/SUCCESS)
int check_secboot();
int check_inodesmap();
int check_datamap();
//logic sectors
int secBootLs();//donde inicia el master boot
int iNodesMapLs();//donde inicia el mapa de bits del area de nodos i
int dataMapLs();//donde inicia el mapa de bits del area de datos
int iNodeLs();//donde inicia el area de nodos i
int dataBlockLs();//donde inicia el area de archivos
/**/
//////////////////////////////////////////////////////////

int check_secboot()
{
	if(!secboot_en_memoria)
	{
		if(vdreadsector(0,0,0,1,1,(char *) &secBoot)== ERROR)
			return ERROR;
		secboot_en_memoria=1;
	}

	return SUCCESS;
}

int check_inodesmap()
{
	if(!inodesmap_en_memoria)
	{
		if (vdreadls(iNodesMapLs(),1,(char *) &iNodesMap) == ERROR) //inicializamos el mapa de i nodes si no existia antes
			return ERROR;
		inodesmap_en_memoria=1;
	}

	return SUCCESS;
}

int check_datamap()
{
	if(!datamap_en_memoria)
	{
		if (vdreadls(dataMapLs(),1, &dataMap) == ERROR) //inicializamos sector boot si no existia antes
			return ERROR;
		datamap_en_memoria=1;
	}

	return SUCCESS;
}

//

int secBootLs()
{
	check_secboot();
  return 0; //secBoot.sec_res;
}

int iNodesMapLs()
{
		check_secboot();
    return secBoot.sec_mapa_bits_nodo_i;
}

int dataMapLs()
{
		check_secboot();
    return secBoot.sec_mapa_bits_bloques;
}

int iNodeLs()
{
		check_secboot();
    return secBoot.sec_tabla_nodos_i;
}

/**
* donde inicia el area de archivos
* @method dataBlockLs
*/
int dataBlockLs()
{
		check_secboot();
    return secBoot.sec_log_unidad;
}
