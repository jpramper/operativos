#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "dataArea.h"

extern short secboot_en_memoria;  //bandera de sec boot
extern struct SECBOOT secBoot;	   // estructura secboot

extern short inodesmap_en_memoria; // inode map flag
extern unsigned char iNodesMap[SECSIZE]; //mapa de bits de iNode

extern short datamap_en_memoria;	//bandera de el mapa de datos
extern unsigned char dataMap[SECSIZE]; // el mapa de datos

// *************************************************************************
// Para el mapa de bits del área de nodos i
// *************************************************************************

int nextfreeinode()
{
	int i,j;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!inodesmap_en_memoria)
	{
		if (vdreadls(inodeMapLs(),1,(char *) &iNodesMap) == -1) //inicializamos el mapa de i nodes si no existia antes
			return -1;
		inodesmap_en_memoria=1;
	}

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

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!inodesmap_en_memoria)
	{
		if (vdreadls(inodeMapLs(),1,(char *) &iNodesMap) == -1) //inicializamos el mapa de i nodes si no existia antes
			return -1;
		inodesmap_en_memoria=1;
	}

	if(iNodesMap[offset] & (1<<shift))
		return(0);  //no ta libre

	return(1);  //si esta libre
}	

int assigninode(int inode)
{
	//printf("esta es mi direccion %x - %d\n", inode, inode);
	int offset=inode/8;
	int shift=inode%8;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!inodesmap_en_memoria)
	{
		if (vdreadls(inodeMapLs(),1,(char *) &iNodesMap) == -1) //inicializamos el mapa de i nodes si no existia antes
			return -1;
		inodesmap_en_memoria=1;
	}

	iNodesMap[offset]|=(1<<shift);
	//printf("este es mi iNode que voy a escibir %x\n", iNodesMap );
	if (vdwritels(inodeMapLs(),1,(char *) &iNodesMap) == -1) //escribimos
			return -1;
	
	return(1);
}

int unassigninode(int inode)
{
	int offset=inode/8;
	int shift=inode%8;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!inodesmap_en_memoria)
	{
		if (vdreadls(inodeMapLs(),1,(char *) &iNodesMap) == -1) //inicializamos el mapa de i nodes si no existia antes
			return -1;
		inodesmap_en_memoria=1;
	}

	iNodesMap[offset]&=(char) ~(1<<shift);
	if (vdwritels(inodeMapLs(),1,(char *) &iNodesMap) == -1) //escribimos
			return -1;

	return(1);
}

// Escribir los datos de un archivo en un nodo i específico
// num es el número de nodo i
// filename el nombre que va a llevar ese archivo en el nodo i
// atribs son los permisos del archivo
// uid id del usuario dueño deñ archivo
// gid id del grupo dueño del archivo
int setninode(int num, char *filename,unsigned short atribs, int uid, int gid)
{
	int i;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!inodesmap_en_memoria)
	{
		if (vdreadls(inodeMapLs(),1,(char *) &iNodesMap) == -1) //inicializamos el mapa de i nodes si no existia antes
			return -1;
		inodesmap_en_memoria=1;
	}
	
	// se establecen los datos
	strncpy(dirRaiz[num].name,filename,20);
	if(strlen(dirRaiz[num].name)>19)
	 	dirRaiz[num].name[19]='\0';
	dirRaiz[num].datetimecreat=currdatetimetoint();
	dirRaiz[num].datetimemodif=currdatetimetoint();
	dirRaiz[num].uid=uid;
	dirRaiz[num].gid=gid;
	dirRaiz[num].perms=atribs;
	dirRaiz[num].size=0;
	
	for(i=0;i<10;i++)
		dirRaiz[num].blocks[i]=0;

	dirRaiz[num].indirect=0;
	dirRaiz[num].indirect2=0;

	// Optimizar la escritura escribiendo solo el sector lógico que
	// corresponde al inodo que estamos asignando.
	// i=num/8;
	// result=vdwritels(inicio_nodos_i+i,&dirRaiz[i*8]);
	for(i=0;i<secBoot.sec_tabla_nodos_i;i++)
		result=vdwritels(inicio_nodos_i+i,&dirRaiz[i*8]);

	return(num);
}

int searchinode(char *filename)
{
	int i;
	int free;

	if(!secboot_en_memoria)
	{
		if (vdreadls(mbrLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
			return -1;
		secboot_en_memoria=1;
	}

	if(!inodesmap_en_memoria)
	{
		if (vdreadls(inodeMapLs(),1,(char *) &iNodesMap) == -1) //inicializamos el mapa de i nodes si no existia antes
			return -1;
		inodesmap_en_memoria=1;
	}
	
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

	unsigned short temp[SECSIZE];

	// Recorrer todos los apuntadores directos del nodo i
	// Poner en 0 en el mapa de bits de bloque, los bloques asignados.
	for(i=0;i<10;i++)
		if(dirRaiz[numinode].blocks[i]!=0)
			unassignblock(dirRaiz[numinode].blocks[i]);

	// Si hay bloque indirecto
	if(dirRaiz[numinode].indirect!=0)
	{
		// Leer el bloque el bloque indirecto a memoria
		readblock(dirRaiz[numinode].indirect,(char *) temp);

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

// ******************************************************************************
// Funciones para el manejo de fechas en los inodos
// ******************************************************************************

unsigned int datetoint(struct DATE date)
{
	unsigned int val=0;

	val=date.year-1970;
	val<<=4;
	val|=date.month;
	val<<=5;
	val|=date.day;
	val<<=5;
	val|=date.hour;
	val<<=6;
	val|=date.min;
	val<<=6;
	val|=date.sec;
	
	return(val);
}

int inttodate(struct DATE *date,unsigned int val)
{
	date->sec=val&0x3F;
	val>>=6;
	date->min=val&0x3F;
	val>>=6;
	date->hour=val&0x1F;
	val>>=5;
	date->day=val&0x1F;
	val>>=5;
	date->month=val&0x0F;
	val>>=4;
	date->year=(val&0x3F) + 1970;
	return(1);
}

unsigned int currdatetimetoint()
{
	struct tm *tm_ptr;
	time_t the_time;
	
	struct DATE now;

	(void) time(&the_time);
	tm_ptr=gmtime(&the_time);

	now.year=tm_ptr->tm_year-70;
	now.month=tm_ptr->tm_mon+1;
	now.day=tm_ptr->tm_mday;
	now.hour=tm_ptr->tm_hour;
	now.min=tm_ptr->tm_min;
	now.sec=tm_ptr->tm_sec;
	return(datetoint(now));
}