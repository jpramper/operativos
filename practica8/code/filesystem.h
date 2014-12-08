
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
int vdwrite();//TODO test
int vdclose();//TODO check
/**/
//////////////////////////////////////////////////////////

struct vddirent *vdreaddir(VDDIR *dirdesc);
VDDIR *vdopendir(char *path);

VDDIR dirs[2]={-1,-1};
struct vddirent dirActual;

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

	//printf("inode buscado: %d\n", numinode);
	if(numinode==-1)
	{
		numinode=nextfreeinode();
		//printf("inode libre siguiente: %d\n", numinode);
		if(numinode==-1)
			return ERROR;
	}
	else {
		removeinode(numinode);
		//printf("inode quitado: %d\n", numinode);
	}

	assigninode(numinode);
	setinode(numinode,filename,perms,getuid(),getgid());

	//////
	//2
	//busca un lugar en la tabla openfiles
	int fd=3; // 0 = stdin

	check_openfiles();

	while(openfiles[fd].inuse && fd<NOPENFILES)
		fd++;
	if(fd>=NOPENFILES)
		return ERROR;

	//printf("lugar en openfiles encontrado: %d\n", fd);

	openfiles[fd].inuse=1;
	openfiles[fd].inode=numinode;
	openfiles[fd].currpos=0;
	openfiles[fd].currbloqueenmemoria=-1;

	//printf("este es el inode de romi: %s\n", );

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

	return SUCCESS;
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
int vdwrite(int fd, char *buffer, int bytes)
{
	check_openfiles();
	if(openfiles[fd].inuse==NO)
		return ERROR;

	int currinode=openfiles[fd].inode;

	int currblock;
	int cont=0;
	int sector;
	unsigned short *currptr;

	while(cont<bytes)
	{
		currptr=currpostoptr(fd);
		if(currptr==NULL)
			return ERROR;

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
			int desplazamiento= currinode * INODESIZE / SECSIZE;//@romi mode
			vdwritels(iNodeLs()+desplazamiento,1,&dirRaiz[currinode/ INODESIZE / SECSIZE]);
			//sector=(currinode/8)*8;//@deprecated
			//result=vdwriteseclog(inicio_nodos_i+sector,&inode[sector*8]);
		}

		//printf("asdf current block %d\n", currblock);

		// Si el bloque de la posición actual no está en memoria
		// Lee el bloque al buffer del archivo
		if(openfiles[fd].currbloqueenmemoria!=currblock)
		{
			//printf("dato que se envia a readblock (=P): %s\n", openfiles[fd].buffer);
			readblock(currblock,openfiles[fd].buffer);
			openfiles[fd].currbloqueenmemoria=currblock;
		}

		// Copia el caracter del buffer que se recibe como argumento
		// al buffer de la tabla de archivos abiertos
		openfiles[fd].buffer[openfiles[fd].currpos%BLOCKSIZE]=buffer[cont];//@tested

		// Incrementa posición
		openfiles[fd].currpos++;

		// Si la posición es mayor que el tamaño, modifica el tamaño
		if(openfiles[fd].currpos>dirRaiz[currinode].size)
			dirRaiz[openfiles[fd].inode].size=openfiles[fd].currpos;

		// Si se llena el buffer, escríbelo
		if(openfiles[fd].currpos%BLOCKSIZE==0)
			writeblock(currblock,openfiles[fd].buffer);

		// Incrementa el contador
		cont++;
	}
	//printf("size del archivo es (terminando write )%d \n", dirRaiz[openfiles[fd].inode].size );

	return(cont);
}

// Les toca hacerla a ustedes
// Debe retornar cuantos caracteres fueron leídos
int vdread(int fd, char *buffer, int bytes)
{
	//printf("size del archivo es (empezando read) %d \n", dirRaiz[openfiles[fd].inode].size );
	// revisar archivos aviertos
	check_openfiles();
	if(openfiles[fd].inuse==NO)
		return ERROR;

	// inicializar estados
	int currinode=openfiles[fd].inode;
	int currblock;
	int cont=0;
	int sector;
	unsigned short *currptr;

	// mientras hayan bytes por leer, y si la posición es menor que el tamaño
	// printf("\ncont : %d\n", cont);
	// printf("ytes : %d\n", bytes);
	// printf("currpos : %d\n", openfiles[fd].currpos);
	// printf("size : %d\n", dirRaiz[currinode].size);
	while(cont < bytes && openfiles[fd].currpos < dirRaiz[currinode].size)
	{
		// busca la siguiente posición del apuntador del archivo
		currptr=currpostoptr(fd);
		if(currptr==NULL)
			return ERROR;

		// El contenido de la dirección currptr es el número de bloque
		currblock=*currptr;
		// Si el bloque de la posición actual no está en memoria
		// Lee el bloque al buffer del archivo
		if(openfiles[fd].currbloqueenmemoria!=currblock)
		{
			readblock(currblock,openfiles[fd].buffer); // lee del fisico a memoria
			//readblock(currblock,buffer); // lee del fisico al buffer
			openfiles[fd].currbloqueenmemoria=currblock;
		}

		// Copia el caracter del buffer de la tabla de archivos abiertos
		// al buffer que se recibe como argumento
		buffer[cont] = openfiles[fd].buffer[openfiles[fd].currpos%BLOCKSIZE];

		// Incrementa posición
		openfiles[fd].currpos++;

		// Incrementa el contador
		cont++;
	}

	return(cont);
}

// Les toca hacerla a ustedes
// Actualizar los nodos-i en disco con respecto al nodo i en memoria
// Si hay bloques de datos que se quedaron a medias en memoria, Escribirlos
int vdclose(int fd)
{

	check_openfiles();
	if(openfiles[fd].inuse==NO)
		return ERROR;

	int currinode=openfiles[fd].inode;

	int currblock;
	//printf("size del archivo es (en close) %d \n", dirRaiz[openfiles[fd].inode].size );
	// actualizar nodos i
	// ---------------------------------------------------------
	assigninode(openfiles[fd].inode);

	// escritura de bloques
	//---------------------------------------------
	// si no termino en un bloque completo, escribe
	if(openfiles[fd].currpos%BLOCKSIZE!=0)
			writeblock(openfiles[fd].currbloqueenmemoria,openfiles[fd].buffer);

	// quitar el archivo de la tabla de archivos abiertos
	openfiles[fd].inuse = NO;


	return SUCCESS;
}

//*************************************************************
//* Manejo de directorios
// *************************************************************

VDDIR *vdopendir(char *path)
{
	int i=0;

	//if(check_secboot()==ERROR) return ERROR; implicit
	if(check_inodesmap()==ERROR) return ERROR;

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

	if(check_inodesmap()==ERROR) return ERROR;

	// Mientras no haya nodo i, avanza
	while(isinodefree(*dirdesc) && *dirdesc<4096)
		(*dirdesc)++;

	// Apunta a donde está el nombre en el inodo
	dirActual.d_name = dirRaiz[*dirdesc].name;

	(*dirdesc)++;

	if(*dirdesc>=4096)
		return(NULL);

	return(&dirActual);
}

int vdclosedir(VDDIR *dirdesc)
{
	(*dirdesc)=-1;
}
