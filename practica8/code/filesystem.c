
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
// Funciones auxiliares en el manejo de archivos
unsigned short *postoptr(int fd,int pos);//TODO test
unsigned short *currpostoptr(int fd);//TODO test
// Funciones del sistema de archivos
int vdcreat(char *filename,unsigned short perms);//TODO test
int vdopen(char *filename,unsigned short mode);//TODO test
int vdunlink(char *filename);//TODO test
int vdseek(int fd, int offset, int whence);//TODO test
int vdread();//TODO check
int vdwrite();//TODO check
int vdclose();//TODO check
/**/
//////////////////////////////////////////////////////////

// ***************************************************************
// Funciones auxiliares en el manejo de archivos
// ***************************************************************
unsigned short *postoptr(int fd,int pos)
{//@return numero de bloque en memoria

	int currinode=openfiles[fd].inode;
	unsigned short *currptr;

	// SI la posicion esta en el rango de los apuntadores directos
	if((pos/BLOCKSIZE)< DIRECTPTRxINODE)
	{
		currptr= &dirRaiz[currinode].blocks[pos/BLOCKSIZE];
		return currptr;
	}

	// SI la posicion esta en el rango de los apuntadores indirectos
	if((pos/BLOCKSIZE)-DIRECTPTRxINODE < PTRxBLOCK )
	{
		// el apuntador indirecto no ha sido asignado
		if(dirRaiz[currinode].indirect==0)
		{
			dirRaiz[currinode].indirect=nextfreeblock();
			if(dirRaiz[currinode].indirect == ERROR) return NULL;
			assignblock(dirRaiz[currinode].indirect);
		}

		currptr= &openfiles[fd].buffindirect[(pos/BLOCKSIZE)-DIRECTPTRxINODE];
		return currptr;
	}

	return NULL;
}

unsigned short *currpostoptr(int fd)
{
	return postoptr(fd,openfiles[fd].currpos);
}

// ***************************************************************
// Funciones del sistema de archivos
// ***************************************************************
int vdcreat(char *filename,unsigned short perms)
{
	//////
	//1
	//si el archivo existe, libera su inode, sino, le asigna uno
	int numinode=searchinode(filename);

	if(numinode==-1)
	{
		numinode=nextfreeinode();
		if(numinode==-1)
			return ERROR;
	}
	else
		removeinode(numinode);

	assigninode(numinode);
	setinode(numinode,filename,perms,getuid(),getgid());

	//////
	//2
	//busca un lugar en la tabla openfiles
	int fd=3;

	check_openfiles();
	while(openfiles[fd].inuse && fd<NOPENFILES)
		fd++;
	if(fd>=NOPENFILES)
		return ERROR;

	openfiles[fd].inuse=1;
	openfiles[fd].inode=numinode;
	openfiles[fd].currpos=0;
	openfiles[fd].currbloqueenmemoria=-1;

	return fd;
}

int vdopen(char *filename,unsigned short mode)
{
	int numinode=searchinode(filename);
	if(numinode==-1)
		return ERROR;

	check_openfiles();

	//busca un lugar en la tabla openfiles
	int fd=3;

	check_openfiles();
	while(openfiles[fd].inuse && fd<NOPENFILES)
		fd++;
	if(fd>=NOPENFILES)
		return ERROR;

	//abre el archivo
	openfiles[fd].inuse=1;
	openfiles[fd].inode=numinode;
	openfiles[fd].currpos=0;

	//carga apuntador indirecto (if any)
	if(dirRaiz[numinode].indirect!=0)
		readblock(dirRaiz[numinode].indirect,(char *) openfiles[fd].buffindirect);

	// carga primer bloque
	unsigned short currblock= *currpostoptr(fd);
	readblock(currblock,openfiles[fd].buffer);
	openfiles[fd].currbloqueenmemoria=currblock;

	return fd;
}

int vdunlink(char *filename)
{
	int numinode;

	numinode=searchinode(filename);
	if(numinode==-1)
		return ERROR;

	removeinode(numinode);
}

int vdseek(int fd, int offset, int whence)
{
	check_openfiles();
	if(openfiles[fd].inuse==NO)
		return ERROR;

	unsigned short oldblock=*currpostoptr(fd);
	unsigned short newblock;

	switch(whence)
	{
		case WHENCE_BEG:
			if(offset<0 || offset>dirRaiz[openfiles[fd].inode].size)
				return ERROR;
			openfiles[fd].currpos=offset;
			break;

		case WHENCE_CUR:
			if(openfiles[fd].currpos+offset>dirRaiz[openfiles[fd].inode].size ||
				openfiles[fd].currpos+offset<0)
				return ERROR;
			openfiles[fd].currpos+=offset;
			break;

		case WHENCE_END:
			if(offset>dirRaiz[openfiles[fd].inode].size || offset<0 )
				return ERROR;
			openfiles[fd].currpos=dirRaiz[openfiles[fd].inode].size-offset;
			break;
	}

	newblock=*currpostoptr(fd);

	if(newblock!=oldblock)
	{
		writeblock(oldblock,openfiles[fd].buffer);
		readblock(newblock,openfiles[fd].buffer);
		openfiles[fd].currbloqueenmemoria=newblock;
	}

	return openfiles[fd].currpos;
}

// Escribir en un archivo abierto
// 	fd = Descriptor del archivo que nos devolvió vdcreat o vdopen
//	buffer = Dirección de memoria donde están los datos a escribir
//	bytes = Cuántos bytes voy a escribir
/*
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
 *************************************************************
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
}*/

int main()
{
	printf("File System v0.1\n");
	printf(".....@Espinosa Romina\n");
	printf(".....@Rojas Ivan\n");
	printf(".....@Ramirez Juan\n");
	return 0;
}
