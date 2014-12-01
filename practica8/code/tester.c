#include "iNode.h"

int main()
{
	struct SECBOOT secBoot;

	if (vdreadls(secBootLs(),1,(char *) &secBoot) == -1) //inicializamos sector boot si no existia antes
		return -1;
	//vdreadsector(0,0,0,1,1,(char *) &secBoot);
	printf("secBootLs() %d\n",secBootLs() );
	printf("sec_mapa_bits_nodo_i: %d\n", secBoot.sec_mapa_bits_nodo_i);
	printf("sec_mapa_bits_bloques: %d\n", secBoot.sec_mapa_bits_bloques);
	printf("sec_tabla_nodos_i: %d\n", secBoot.sec_tabla_nodos_i);
	printf("sec_log_unidad: %d\n", secBoot.sec_log_unidad);
	printf("sec_x_bloque: %d\n", secBoot.sec_x_bloque);
	printf("heads: %d\n", secBoot.heads);
	printf("cyls: %d\n", secBoot.cyls);

	printf("tamaño de dir raiz: %d\n", SECSIZE / sizeof(struct INODE) * 8);

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

	return 0;
}
