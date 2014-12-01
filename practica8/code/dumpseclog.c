#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vdisk.h"

#define LINESIZE 16
#define SECSIZE 512

int main(int argc,char *argv[]){
	int sl = atoi(argv[1]);
	int drive = 0;
	int ncyl = sl / (SECTORS * HEADS);
	int nhead = (sl / SECTORS) % HEADS;
	int nsec = (sl % SECTORS) + 1;

	//printf("aahhh osea que buscas: %d, %d, %d, %d" , drive,ncyl, nhead, nsec);
	dumpsec(drive,ncyl,nhead,nsec);
}
int dumpsec(int drive, int ncyl, int nhead, int nsec)
{
	int fd;
	unsigned char buffer[SECSIZE];
	int offset;
	int i,j,r;
	unsigned char c;

	if(vdreadsector(drive,nhead,ncyl,nsec,1,buffer)==-1)
	{
		fprintf(stderr,"Error al abrir disco virtual\n");
		exit(1);
	}
	for(i=0;i<SECSIZE/LINESIZE;i++)
	{
		printf("\n %3X -->",i*LINESIZE);
		for(j=0;j<LINESIZE;j++)
		{
			c=buffer[i*LINESIZE+j];
			printf("%2X ",c);
		}
		printf(" | ");
		for(j=0;j<LINESIZE;j++)
		{
			c=buffer[i*LINESIZE+j]%256;
			if(c>0x1F && c<127)
				printf("%c",c);
			else
				printf(".");
		}
	}
	printf("\n");
}
