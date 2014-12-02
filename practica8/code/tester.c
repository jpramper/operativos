#include "iNode.h"

void test_logicsectors();
void test_assigninode();
void test_assignblock();
void test_writeblock();

int main()
{
	test_logicsectors();
	test_assigninode();
	//test_assignblock();
	test_writeblock();

	return 0;
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

	unsigned char buffer[SECSIZE*4];

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

	unsigned char buffer2[SECSIZE*4];

	readblock(block,buffer2);
	writeblock(block2,buffer2);

}
