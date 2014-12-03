#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "dataArea.h"
#include "date.h"

extern short secboot_en_memoria;  // sec boot
extern struct SECBOOT secBoot;
extern short inodesmap_en_memoria; // inode
extern unsigned char iNodesMap[SECSIZE];
extern short datamap_en_memoria;	//mapa de datos
extern unsigned char dataMap[SECSIZE];

//////////////////////////////////////////////////////////
/*Method Declaration Area*/
// Para el mapa de bits del área de nodos i
int nextfreeinode();
int isinodefree(int inode);
int assigninode(int inode);
int unassigninode(int inode);
/**/
//////////////////////////////////////////////////////////

// *************************************************************************
// Para el mapa de bits del área de nodos i
// *************************************************************************

int nextfreeinode()
{
	int i,j;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_inodesmap()==ERROR) return ERROR;

	// Recorrer byte por byte mientras sea 0xFF sigo recorriendo
	// i encuentra el byte (offset)
	// j recorre el byte para encontrar el bit libre (shift)
	i=0;
	while(iNodesMap[i]==0xFF && i<secBoot.sec_mapa_bits_nodo_i*SECSIZE)
		i++;

	//printf("mi primer nodo i libre es: %x, %d \n",i,i);
	//printf("yo sec_mapa_bits_nodo_i valgo : %d \n", secBoot.sec_mapa_bits_nodo_i);
	if(i<secBoot.sec_mapa_bits_nodo_i*SECSIZE)
	{
		j=0;
		while(iNodesMap[i] & (1<<j) && j<8)
			j++;

		return(i*8+j);
	}

	// si te pasaste error
	return(-1);
}

int isinodefree(int inode)  //retrun 1 si esta libre
							//0 si no
{
	int offset=inode/8;
	int shift=inode%8;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_inodesmap()==ERROR) return ERROR;

	if(iNodesMap[offset] & (1<<shift))
		return(0);  //no ta libre

	return(1);  //si esta libre
}

int assigninode(int inode)
{
	//printf("esta es mi direccion %x - %d\n", inode, inode);
	int offset=inode/8;
	int shift=inode%8;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_inodesmap()==ERROR) return ERROR;

	iNodesMap[offset]|=(1<<shift);
	//printf("este es mi iNode que voy a escibir %x\n", iNodesMap );
	if (vdwritels(iNodesMapLs(),1,(char *) &iNodesMap) == -1) //escribimos
			return -1;

	return(1);
}

int unassigninode(int inode)
{
	int offset=inode/8;
	int shift=inode%8;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_inodesmap()==ERROR) return ERROR;

	iNodesMap[offset]&=(char) ~(1<<shift);
	if (vdwritels(iNodesMapLs(),1,(char *) &iNodesMap) == -1) //escribimos
			return -1;

	return(1);
}

// *************************************************************************
//TODO
// *************************************************************************

int setinode(int num, char *filename, unsigned short atribs, int uid, int gid)
{
	int i;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_dirraiz()==ERROR) return ERROR;
	
	// se establecen los datos
	strncpy(dirRaiz[num].name,filename,20);
	if(strlen(dirRaiz[num].name)>19)
	 	dirRaiz[num].name[19]='\0';

	dirRaiz[num].datetimecreate = currdatetimetoint();
	dirRaiz[num].datetimemodif = currdatetimetoint();
	dirRaiz[num].uid=uid;
	dirRaiz[num].gid=gid;
	dirRaiz[num].perms=atribs;
	dirRaiz[num].size=0;

	for(i=0;i<BLOCKPTRxINODE;i++)
		dirRaiz[num].blocks[i]=0;

	dirRaiz[num].indirect=0;
	dirRaiz[num].indirect2=0;

	// escribe el sector donde se encuentra el inode de dirRaiz[num]
	for(i=0;i<secBoot.sec_tabla_nodos_i;i++)
		vdwritels(iNodeLs()+i,1,&dirRaiz[i*8]);
	// TODO Optimizar @romi

	return(num);
}

int searchinode(char *filename)
{
	int i;
	//Xint free;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_inodesmap()==ERROR) return ERROR;

	// Si el nombre del archivo sobrepasa los 19 bytes, truncarlo
	if(strlen(filename)>19)
	  	filename[19]='\0';

	// Buscar en toda la tabla de nodos i, el archivo que queremos
	// encontrar
	i=0;
	while(strcmp(dirRaiz[i].name,filename) && i<NINODES)
		i++;

	// Si i llegó a 64, el archivo buscado no existe, regresar -1 (error)
	if(i>=NINODES)
		return(-1);

	return(i);
}

// Borrar un nodo i de la tabla de nodos i.
int removeinode(int numinode)
{
	int i;
	unsigned char temp[SECSIZE*SECxBLOCK];

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_inodesmap()==ERROR) return ERROR;

	// se establecen los datos
	strncpy(dirRaiz[numinode].name,"\0",20);

	// escribe el sector donde se encuentra el inode de dirRaiz[num]
	for(i=0;i<secBoot.sec_tabla_nodos_i;i++)
		vdwritels(iNodeLs()+i,1,&dirRaiz[i*8]);
	// TODO Optimizar @romi

	// Recorrer todos los apuntadores directos del nodo i
	// Poner en 0 en el mapa de bits de bloque, los bloques asignados.
	for(i=0;i<BLOCKPTRxINODE;i++)
		if(dirRaiz[numinode].blocks[i]!=0)
			unassignblock(dirRaiz[numinode].blocks[i]);

	// Si hay bloque indirecto
	if(dirRaiz[numinode].indirect!=0)
	{
		// Leer el bloque el bloque indirecto a memoria
		readblock(dirRaiz[numinode].indirect, temp);

		// Recorrer todos los apuntadores que contiene el bloque
		// y poner en 0s su bit correspondiente en el mapa de bits
		for(i=0;i< (SECSIZE*SECxBLOCK);i++)
			if(temp[i]!=0)
				unassignblock(temp[i]);

		// Desasignar en el mapa de bits el bloque indirecto
		// es decir, el bloque de apuntadores
		unassignblock(dirRaiz[numinode].indirect);
		dirRaiz[numinode].indirect=0;
	}
	
	// Desasignar el nodo i, en el mapa de bits
	unassigninode(numinode);

	return(1);
}
