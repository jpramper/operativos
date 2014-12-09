#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "filesystem.h"

void locateend(char *cmd);
int executecmd(char *cmd);

void dumphelp();

int main()
{
 	printf(".\n.\nFile System v2.7\n");
 	printf(".....@Espinosa Romina\n");
 	printf(".....@Ramirez Juan\n");
 	printf(".....@Rojas Ivan (el Q)\n.\n");
	char linea[MAXLEN];
	int result=1;

	while(result)
	{
		printf("vshell > ");
		fflush(stdout);
		read(0,linea,80);
		locateend(linea);
		result=executecmd(linea);

		//printf("resutl: %d\n", result );
	} 
}
void locateend(char *linea)
{
// Localiza el fin de la cadena para poner el fin
	int i=0;
	while(i<MAXLEN && linea[i]!='\n')
		i++;
	linea[i]='\0';
}
int executecmd(char *linea)
{
	if(strlen(linea) <=0)
		return 1;

	char *cmd;
	char *arg1;
	char *arg2;
	char *search=" ";
// Separa el comando y los dos posibles argumentos
	cmd=strtok(linea," ");
	arg1=strtok(NULL," ");
	arg2=strtok(NULL," ");
// comando "exit"
	if(strcmp(cmd,"exit")==0)
		return(0);
// comando "copy"
	if(strcmp(cmd,"copy")==0)
	{
		if(arg1==NULL || arg2==NULL)
		{
			fprintf(stderr,"Error en los argumentos\n");
			return(1);
		}

		if(!isinvd(arg1) && !isinvd(arg2))
			copyuu(&arg1[2],&arg2[2]);
		else if(!isinvd(arg1) && isinvd(arg2))
			copyuv(&arg1[2],arg2);
		else if(isinvd(arg1) && !isinvd(arg2))
			copyvu(arg1,&arg2[2]);
		else if(isinvd(arg1) && isinvd(arg2))
			copyvv(arg1,arg2);
	return 1;
	}
// comando "create"
	if(strcmp(cmd,"create")==0)
	{
		if(arg1==NULL)
		{
			fprintf(stderr,"Error en los argumentos\n");
			return(1);
		}

		if(isinvd(arg1))
			createv(arg1);
		else
			createu(&arg1[2]);
		
		return 1;
	}

// comando "cat"
	if(strcmp(cmd,"cat")==0)
	{
		if(arg1==NULL)
		{
			fprintf(stderr,"Error en los argumentos\n");
			return(1);
		}

		if(isinvd(arg1))
			catv(arg1);
		else
			catu(&arg1[2]);
		
		return 1;
	}
// comando "help"
	if(strcmp(cmd,"help")==0)
	{
		dumphelp();
		return 1;
	}
// comando "delete"
	if(strcmp(cmd,"delete")==0)
	{
		if(arg1==NULL)
		{
			fprintf(stderr,"Error en los argumentos\n");
			return(1);
		}

		if(isinvd(arg1))
			deletev(arg1);
		else
			deleteu(&arg1[2]);
		
		return 1;
	}
// comando dir
	if(strcmp(cmd,"dir")==0)
	{
		if(arg1==NULL)
			dirv();
		else if(!isinvd(arg1))
			diru(&arg1[2]);
		return 1;
	}
}
/* Regresa verdadero si el nombre del archivo no comienza con // y por lo 
 tanto es un archivo que está en el disco virtual */
int isinvd(char *arg)
{
	if(strncmp(arg,"//",2)!=0)
		return(1);
	else
		return(0);
}
/* Copia un archivo del sistema de archivos de UNIX a un archivo destino
 en el mismo sistema de archivos de UNIX */
int copyuu(char *arg1,char *arg2)
{
	int sfile,dfile;
	char buffer[BUFFERSIZE];
	int ncars;
	sfile=open(arg1,0);
	dfile=creat(arg2,0640);
	do {
		ncars=read(sfile,buffer,BUFFERSIZE);
		write(dfile,buffer,ncars);
	} while(ncars==BUFFERSIZE);
	close(sfile);
	close(dfile);
	return(1);
}
/* Copia un archivo del sistema de archivos de UNIX a un archivo destino
en el el disco virtual */
int copyuv(char *arg1,char *arg2)
{
	int sfile,dfile;
	char buffer[BUFFERSIZE];
	int ncars;
	sfile=open(arg1,0);
	dfile=vdcreat(arg2,0640);
	do {
		ncars=read(sfile,buffer,BUFFERSIZE);
		vdwrite(dfile,buffer,ncars);
	} while(ncars==BUFFERSIZE);
	close(sfile);
	vdclose(dfile);
	return(1);
}
/* Copia un archivo del disco virtual a un archivo destino
 en el sistema de archivos de UNIX */
int copyvu(char *arg1,char *arg2)
{
	int sfile,dfile;
	char buffer[BUFFERSIZE];
	int ncars;
	sfile=vdopen(arg1,0);
	dfile=creat(arg2,0640);
	do {
		ncars=vdread(sfile,buffer,BUFFERSIZE);
		//printf("buffer de nuestro archivo: %s\n",buffer);
		write(dfile,buffer,ncars);
	} while(ncars==BUFFERSIZE);
	vdclose(sfile);
	close(dfile);
	return(1);
}
/* Copia un archivo del disco virtual a un archivo destino
 en el mismo disco virtual */
int copyvv(char *arg1,char *arg2)
{
	int sfile,dfile;
	char buffer[BUFFERSIZE];
	int ncars;
	sfile=vdopen(arg1,0);
	dfile=vdcreat(arg2,0640);
	do {
		ncars=vdread(sfile,buffer,BUFFERSIZE);
		vdwrite(dfile,buffer,ncars);
	} while(ncars==BUFFERSIZE);
	vdclose(sfile);
	vdclose(dfile);
	return(1);
}
/* Despliega un archivo del disco virtual a pantalla */
int catv(char *arg1)
{
	int sfile,dfile;
	char buffer[BUFFERSIZE];
	int ncars;
	sfile=vdopen(arg1,0);

	if(sfile==ERROR)
	{
		printf("Error %s no existe.\n",arg1);
		return 1;
	}

	do {
		ncars+=vdread(sfile,buffer,BUFFERSIZE);
		write(1,buffer,ncars); // Escribe en el archivo de salida estandar
	} while(ncars<=BUFFERSIZE);
	vdclose(sfile);
	return(1);
}
/* Despliega un archivo del sistema de archivos de UNIX a pantalla */
int catu(char *arg1)
{
	int sfile,dfile;
	char buffer[BUFFERSIZE];
	int ncars;
	sfile=open(arg1,0);
	do {
		ncars=read(sfile,buffer,BUFFERSIZE);
		write(1,buffer,ncars); // Escribe en el archivo de salida estandar
	} while(ncars==BUFFERSIZE);
	close(sfile);
	return(1);
}
/* Muestra el directorio en el sistema de archivosd de UNIX */
int diru(char *arg1)
{
	DIR *dd;
	struct dirent *entry;
	if(arg1[0]=='\0')
		strcpy(arg1,".");
	printf("Directorio %s\n",arg1);
	dd=opendir(arg1);
	if(dd==NULL)
	{
		fprintf(stderr,"Error al abrir directorio\n");
		return(-1);
	}
	while((entry=readdir(dd))!=NULL)
		printf("%s\n",entry->d_name);
	closedir(dd);
}
/* Muestra el directorio en el sistema de archivos en el disco virtual */
int dirv()
{
	int i;
	char *filename;
	for(i=0; i<NINODES; i++)
	{
		filename= getfilename(i);
		if(filename!=NULL)
			printf("\t./%s\n",filename);
	}
}

int deleteu(char *arg1)
{
	return unlink(arg1);
}

int deletev(char *arg1)
{
	return vdunlink(arg1);

}

int createv(char *arg1)
{
	int file= vdcreat(arg1,0);
	vdclose(file);
	return(1);
}

int createu(char *arg1)
{
	int file= creat(arg1,777);
	close(file);
	return(1);
}

void dumphelp()
{
	printf(".\n.\nFile System v2.7.\n");
 	printf(".....@Espinosa Romina\n");
 	printf(".....@Ramirez Juan\n");
 	printf(".....@Rojas Ivan (el Q)\n.\n.");

 	printf("\n---Comandos disponibles---\n");
 	printf("\t$ create (fileName)\n");
 	printf("\t$ copy (source) (target)\n");
 	printf("\t$ cat (fileName)\n");
 	printf("\t$ delete (fileName)\n");
 	printf("\t$ dir (directoryName)\n");
 	printf("\t$ * // to reference unix file system\n.\n.\n");
}