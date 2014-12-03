
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "iNode.h"

extern short secboot_en_memoria;  // sec boot
extern struct SECBOOT secBoot;
extern short inodesmap_en_memoria; // inode
extern unsigned char iNodesMap[SECSIZE];
extern short datamap_en_memoria;	//mapa de datos
extern unsigned char dataMap[SECSIZE];

//////////////////////////////////////////////////////////
/*Method Declaration Area*/
int vdcreat();
int vdopen();
int vdread();
int vdwrite();
int vdseek();
int vdclose();
int vdunlink();

int isinodefree(int inode);
int assigninode(int inode);
int unassigninode(int inode);
/**/
//////////////////////////////////////////////////////////

// ***************************************************************
// Funciones del sistema de archivos
// ***************************************************************

// Tabla de archivos abiertos
int openfiles_inicializada=0;
struct OPENFILES openfiles[16];


// Funciones auxiliares en el manejo de archivos
unsigned short *postoptr(int fd,int pos)
{
	int currinode;
	unsigned short *currptr;
	unsigned short indirect1;

	// Obtener el nodo I actual de la tabla de archivos abiertos
	currinode=openfiles[fd].inode;

	// Está en los primeros 20 K
	if((pos/1024)<20)	// De 0 a 20479
		// Está entre los 10 apuntadores directos
		currptr=&inode[currinode].blocks[pos/2048];
	else if((pos/1024)<2048+20)
	{
		// Si el indirecto está vacío, asígnale un bloque
		if(inode[currinode].indirect==0)
		{
			// El primer bloque disponible
			indirect1=nextfreeblock();
			assignblock(indirect1); // Asígnalo
			inode[currinode].indirect=indirect1;
		} 
		currptr=&openfiles[fd].buffindirect[pos/2048-20];
	}
	else
		return(NULL);

	return(currptr);
}



unsigned short *currpostoptr(int fd)
{
	unsigned short *currptr;

	// openfiles[fd].currpos Es la posición actual del puntero del 
	// archivo
	currptr=postoptr(fd,openfiles[fd].currpos);

	return(currptr);
}



// Funciones para crear, abrir y eliminar archivos

int vdopen(char *filename,unsigned short mode)
{
	int numinode;
	int i;
	unsigned short currblock;

	// Ver si ya existe el archivo
	// Si no existe regresa con un error
	numinode=searchinode(filename);
	if(numinode==-1)
		return(-1);

	// Si no está inicializada la tabla de archivos abiertos inicialízala
	if(!openfiles_inicializada)
	{
		for(i=3;i<16;i++)
		{
			openfiles[i].inuse=0;
			openfiles[i].currbloqueenmemoria=-1;
		}
		openfiles_inicializada=1;
	}

	// Buscar si hay lugar en la tabla de archivos abiertos
	// Si no hay lugar, regresa -1
	i=3;
	while(openfiles[i].inuse && i<16)
		i++;

	if(i>=16)
		return(-1);

	// Si hay lugar
	openfiles[i].inuse=1;
	openfiles[i].inode=numinode;
	openfiles[i].currpos=0;

	// Si hay apuntador indirecto, leerlo en el buffer
	if(inode[numinode].indirect!=0)
		readblock(inode[numinode].indirect,(char *) openfiles[i].buffindirect);

	// Cargar el buffer con el bloque actual del archivo (primer bloque)
	currblock=*currpostoptr(i);
	readblock(currblock,openfiles[i].buffer);
	openfiles[i].currbloqueenmemoria=currblock;
	return(i);
}


int vdcreat(char *filename,unsigned short perms)
{
	int numinode;
	int i;

	// Ver si ya existe el archivo
	numinode=searchinode(filename);

	// Si el archivo aún no existe
	if(numinode==-1)
	{
		// Buscar un inodo en blanco en el mapa de bits (nodos i)
		numinode=nextfreeinode();
		if(numinode==-1)
		{
			return(-1); // No hay espacio para más archivos
		}
	} else	// Si el archivo ya existe, elimina el inodo
		removeinode(numinode);


	// Escribir el archivo en el inodo encontrado

	setninode(numinode,filename,perms,getuid(),getgid());
	assigninode(numinode);


	if(!openfiles_inicializada)
	{
		for(i=3;i<16;i++)
		{
			openfiles[i].inuse=0;	// Archivo no está en uso
			openfiles[i].currbloqueenmemoria=-1; // Ningún bloque
		}
		openfiles_inicializada=1;
	}

	// Buscar si hay lugar en la tabla de archivos abiertos
	// Si no hay lugar, regresa -1
	i=3;
	while(openfiles[i].inuse && i<16)
		i++;

	// Si llegué al final de la tabla y no hay lugar para el archivo
	if(i>=16)
		return(-1);

	// Poner el archivo en la tabla de archivos abiertos
	openfiles[i].inuse=1;
	openfiles[i].inode=numinode;
	openfiles[i].currpos=0;
	openfiles[i].currbloqueenmemoria=-1;
	return(i);
}


int vdunlink(char *filename)
{
	int numinode;
	int i;

	// Busca el inodo del archivo
	numinode=searchinode(filename);
	if(numinode==-1)
		return(-1); // No existe

	removeinode(numinode);
}



// Para movernos dentro de un archivo
//	fd = Descriptor del archivo en la tabla de archivos abiertos
//	offset = cuanto me voy a mover
//	whence = a partir de donde
//		0 – A partir del inicio del archivo
//		1 – A partir de la posición actual del puntero
//		2 – A partir del final del archivo
//	La función regresa la posición final del puntero después del vdseek
int vdseek(int fd, int offset, int whence)
{
	unsigned short oldblock,newblock;

	// Si no está abierto regresa error
	if(openfiles[fd].inuse==0)
		return(-1);

	oldblock=*currpostoptr(fd);
		
	if(whence==0) // A partir del inicio
	{
		if(offset<0 || 
		   openfiles[fd].currpos+offset>inode[openfiles[fd].inode].size)
			return(-1);
		openfiles[fd].currpos=offset;

	} else if(whence==1) // A partir de la posición actual
	{
		if(openfiles[fd].currpos+offset>inode[openfiles[fd].inode].size ||
		   openfiles[fd].currpos+offset<0)
			return(-1);
		openfiles[fd].currpos+=offset;

	} else if(whence==2) // A partir del final
	{
		if(offset>inode[openfiles[fd].inode].size ||
		   openfiles[fd].currpos-offset<0)
			return(-1);
		openfiles[fd].currpos=inode[openfiles[fd].inode].size-offset;
	} else
		return(-1);

	newblock=*currpostoptr(fd);
	
	if(newblock!=oldblock)
	{
		writeblock(oldblock,openfiles[fd].buffer);
		readblock(newblock,openfiles[fd].buffer);
		openfiles[fd].currbloqueenmemoria=newblock;
	}

	return(openfiles[fd].currpos);
}

// Escribir en un archivo abierto
// 	fd = Descriptor del archivo que nos devolvió vdcreat o vdopen
//	buffer = Dirección de memoria donde están los datos a escribir
//	bytes = Cuántos bytes voy a escribir
int vdwrite(int fd, char *buffer, int bytes)
{
	int currblock;
	int currinode;
	int cont=0;
	int sector;
	int i;
	int result;
	unsigned short *currptr;

	// Si no está abierto, regresa error
	if(openfiles[fd].inuse==0)
		return(-1);

	// Obtener el inodo del archivo
	currinode=openfiles[fd].inode;

	// Realizar la escritura de cada uno de los bytes del buffer
	// al archivo
	while(cont<bytes)
	{
		// Obtener la dirección de donde está el bloque que corresponde
		// a la posición actual
		currptr=currpostoptr(fd);

		// Si apunta a nulo, error
		if(currptr==NULL)
			return(-1);
	
		// El contenido de la dirección currptr es el número de bloque
		currblock=*currptr;

		// Si el bloque está en blanco, asignar un bloque
		if(currblock==0)
		{
			// Buscar en el mapa de bits el siguiente bloque libre
			currblock=nextfreeblock();
			// El bloque encontrado ponerlo en donde
			// apunta el apuntador al bloque actual
			*currptr=currblock;

			// Poner el bloque encontrado como ocupado
			assignblock(currblock);	
			
			// Escribir el sector de la tabla de nodos i
			// En el disco
			sector=(currinode/8)*8;
			result=vdwriteseclog(inicio_nodos_i+sector,&inode[sector*8]);
		}

		// Si el bloque de la posición actual no está en memoria
		// Lee el bloque al buffer del archivo
		if(openfiles[fd].currbloqueenmemoria!=currblock)
		{
			// Leer el bloque actual hacia el buffer que
			// está en la tabla de archivos abiertos
			readblock(currblock,openfiles[fd].buffer);			
			openfiles[fd].currbloqueenmemoria=currblock;
		}

		// Copia el caracter del buffer que se recibe como argumento
		// al buffer de la tabla de archivos abiertos
		openfiles[fd].buffer[openfiles[fd].currpos%2048]=buffer[cont];

		// Incrementa posición
		openfiles[fd].currpos++;

		// Si la posición es mayor que el tamaño, modifica el tamaño
		if(openfiles[fd].currpos>inode[currinode].size)
			inode[openfiles[fd].inode].size=openfiles[fd].currpos;

		// Incrementa el contador
		cont++;

		// Si se llena el buffer, escríbelo
		if(openfiles[fd].currpos%2048==0)
			writeblock(currblock,openfiles[fd].buffer);
	}
	return(cont);
} 

int vdread(int fd, char *buffer, int bytes)
{
	// Les toca hacerla a ustedes
	// Debe retornar cuantos caracteres fueron leídos
} 


int vdclose(int fd)
{
	// Les toca hacerla a ustedes
	// Actualizar los nodos-i en disco con respecto al nodo i en memoria
	// Si hay bloques de datos que se quedaron a medias en memoria 
	// Escribirlos
}

/*************************************************************
/* Manejo de directorios
 *************************************************************/
VDDIR dirs[2]={-1,-1};
struct vddirent current;


VDDIR *vdopendir(char *path)
{
	int i=0;
	int result;

	if(!secboot_en_memoria)
	{
		result=vdreadsector(0,0,0,1,1,(char *) &secboot);
		secboot_en_memoria=1;
	}
	inicio_nodos_i=secboot.sec_res+secboot.sec_mapa_bits_nodos_i+secboot.sec_mapa_bits_bloques;

	if(!nodos_i_en_memoria)
	{
		for(i=0;i<secboot.sec_tabla_nodos_i;i++)
			result=vdreadseclog(inicio_nodos_i+i,&inode[i*8]);

		nodos_i_en_memoria=1;
	}

	if(strcmp(path,".")!=0)
		return(NULL);

	i=0;
	while(dirs[i]!=-1 && i<2)
		i++;

	if(i==2)
		return(NULL);

	dirs[i]=0;

	return(&dirs[i]);	
}

struct vddirent *vdreaddir(VDDIR *dirdesc)
{
	int i;

	int result;
	if(!nodos_i_en_memoria)
	{
		for(i=0;i<secboot.sec_tabla_nodos_i;i++)
			result=vdreadseclog(inicio_nodos_i+i,&inode[i*8]);

		nodos_i_en_memoria=1;
	}

	// Mientras no haya nodo i, avanza
	while(isinodefree(*dirdesc) && *dirdesc<4096)
		(*dirdesc)++;


	// Apunta a donde está el nombre en el inodo	
	current.d_name=inode[*dirdesc].name;

	(*dirdesc)++;

	if(*dirdesc>=4096)
		return(NULL);
	return( &current);	
}

int vdclosedir(VDDIR *dirdesc)
{
	(*dirdesc)=-1;
}



