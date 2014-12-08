
#include <unistd.h> // manejo de archivos

#include "filesystem.h"

void test_logicsectors();
void test_assigninode();
void test_assignblock();
void test_writeblock();
void test_setinode();
void test_vdcreate();
void test_vdwrite();

int main()
{
	unlink("disco0.vd");
	system("./createvd 0");
	system("./vdformat 0");

	//test_logicsectors();
	//test_assigninode();
	//test_assignblock();
	//test_writeblock();
	//test_setinode();
	//test_vdcreate();
	test_vdwrite();

	system("./dumpseclog 3");
	system("./dumpseclog 4");
	system("./dumpseclog 5");
	system("./dumpseclog 6");
	system("./dumpseclog 7");
	system("./dumpseclog 8");
	system("./dumpseclog 9");
	system("./dumpseclog 10");

	return 0;
}

void test_vdwrite()
{
	// crea el archivo
	printf("\n\n--------------------\n");
	printf("vdwrite test\n");
	printf("--------------------\n\n");
	char *filename = "omg so write";
	unsigned short perms = 1;

	int fd;
	fd = vdcreat(filename,perms);
	printf("este es nuestro nuevo fd: %d\n",fd);

	vdwrite(fd, "una puerca pescuesicrespa", 25);

	vdclose(fd);
}

// *************************************************************************
// TEST AREA
// *

void test_logicsectors()
{
	printf("\n\n--------------------\n");
	printf("logic sectors test\n");
	printf("--------------------\n\n");

	printf("secBootLs() %d\n",secBootLs() );
	printf("iNodesMapLs(): %d\n", iNodesMapLs());
	printf("dataMapLs(): %d\n", dataMapLs());
	printf("iNodeLs(): %d\n", iNodeLs());
	printf("dataBlockLs(): %d\n", dataBlockLs());

	printf("sec_x_bloque: %d\n", secBoot.sec_x_bloque);
	printf("heads: %d\n", secBoot.heads);
	printf("cyls: %d\n", secBoot.cyls);

	printf("tamaño struct SECBOOT: %d\n", SECSIZE / sizeof(struct INODE) * 8);
}

void test_assigninode()
{
	printf("\n\n--------------------\n");
	printf("assign inode test\n");
	printf("--------------------\n\n");

	assigninode(nextfreeinode());
	assigninode(nextfreeinode());
	assigninode(nextfreeinode());
	assigninode(nextfreeinode());

	int i = nextfreeinode();
	printf("nodo asignado: %d\n", i);
	assigninode(i);

	printf("esta libre? %d\n", isinodefree(i));

	printf("desasignado %d\n", i);
	unassigninode(i);

	printf("esta libre? %d\n", isinodefree(i));
}

void test_assignblock()
{
	printf("\n\n--------------------\n");
	printf("assign block test\n");
	printf("--------------------\n\n");

	assignblock(nextfreeblock());

	int i = nextfreeblock();
	printf("bloque asignado: %d\n", i);
	assignblock(i);

	printf("esta libre? %d\n", isblockfree(i));

	printf("desasignado %d\n", i);
	unassignblock(i);

	printf("esta libre? %d\n", isblockfree(i));

}

void test_writeblock()
{
	printf("\n\n--------------------\n");
	printf("write block test\n");
	printf("--------------------\n\n");

	int block = nextfreeblock();
	printf("bloque asignado: %d\n", block);
	assignblock(block);

	unsigned char buffer[SECSIZE*SECxBLOCK];

	int i;
	for (i = 0; i < SECSIZE*4; i++)
		buffer[i] = i%2;

	writeblock(block,buffer);

	//se salta un bloque
	assignblock(nextfreeblock());

	//copia el contenido del primer bloque al siguiente
	int block2 = nextfreeblock();
	printf("segundo bloque asignado: %d\n", block2);
	assignblock(block2);

	unsigned char buffer2[SECSIZE*SECxBLOCK];

	readblock(block,buffer2);
	writeblock(block2,buffer2);

}

void test_setinode()
{
	printf("\n\n--------------------\n");
	printf("setinode test\n");
	printf("--------------------\n\n");

	assigninode(nextfreeinode());
	assigninode(nextfreeinode());
	assigninode(nextfreeinode());
	assigninode(nextfreeinode());

	int i = nextfreeinode();
	printf("inodo asignado: %d\n", i);
	assigninode(i);

	printf("esta libre? %d\n", isinodefree(i));

	printf("nodo establecido: %d\n", i);
	setinode(i, "archivo1", 1, 2, 3);

	int j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo2", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo3", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo4", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo5", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo6", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo7", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo8", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo9", 1, 2, 3);
	j = nextfreeinode();
	assigninode(j);
	setinode(j, "archivo10", 1, 2, 3);

	printf("removed %d\n", i);
	removeinode(i);
	int s = searchinode("archivo1");
	printf("encontrado = %d\n", s);

	printf("esta libre? %d\n", isinodefree(s));

}

void test_vdcreate()
{
	printf("\n\n--------------------\n");
	printf("vdcreat test\n");
	printf("--------------------\n\n");
	char *filename = "fisher price";
	char *filename2 = "huevit";
	char *filename3 = "el gran sayaman!";
	unsigned short perms = 1;

	int fd;
	fd = vdcreat(filename,perms);
	printf("este es nuestro nuevo fd: %d\n",fd);
	fd = vdcreat(filename3,perms);
	printf("este es nuestro nuevo fd: %d\n",fd);
	fd = vdcreat(filename2,perms);
	printf("este es nuestro nuevo fd: %d\n",fd);
}