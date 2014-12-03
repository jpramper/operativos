#define HEADS 16   //superficies
#define SECTORS 11
#define CYLINDERS 100
#define SECSIZE 512
#define SECxBLOCK 4
#define NINODES SECSIZE / sizeof(struct INODE) * 8
#define BLOCKPTRxINODE 10
#define NOPENFILES 10

#define ERROR	-1
#define SUCCESS	1

struct SECBOOT{
	char jump[4];
	char nombre_disco[8];
	unsigned char sec_res;   //reservado
	unsigned char sec_mapa_bits_nodo_i;
	unsigned char sec_mapa_bits_bloques;
	unsigned short sec_tabla_nodos_i;
	unsigned short sec_log_unidad; //
	unsigned char sec_x_bloque;
	unsigned char heads;
	unsigned char cyls;
	unsigned char secfis; // sectores por superficie
	char restante[487];
};

struct INODE{
	char name[20];
	unsigned short uid;   // user ID
	unsigned short gid;   // group ID
	unsigned short perms;  //permisos
	unsigned int datetimecreate;  // fecha de creacion
	unsigned int datetimemodif; // fecha de modificacion
	unsigned int size;			// size del archivo en bytes
	unsigned short blocks[BLOCKPTRxINODE];
	unsigned short indirect;
	unsigned short indirect2;
};

struct OPENFILES {
	int inuse;
	unsigned short inode;
	int currpos;
	int currbloqueenmemoria;
	char buffer[2048];
	unsigned short buffindirect[1024];
};

// typedef int VDDIR;	

// struct vddirent 
// {
// 	char *d_name;
// };

// struct vddirent *vdreaddir(VDDIR *dirdesc);
// VDDIR *vdopendir(char *path);

short secboot_en_memoria = 0;  //bandera de sec boot
struct SECBOOT secBoot;	   // estructura secboot

short inodesmap_en_memoria = 0; // inode map flag
unsigned char iNodesMap[SECSIZE]; //mapa de bits de iNode

short datamap_en_memoria = 0;	//bandera de el mapa de datos
unsigned char dataMap[SECSIZE]; // el mapa de datos

short dirraiz_en_memoria = 0;	//bandera de el mapa de datos
struct INODE dirRaiz[NINODES]; // directorio raiz de inodos
// num de inodos que caben en un sector, por 8 sectores

struct OPENFILES openfiles[NOPENFILES]; // directorio raiz de inodos
