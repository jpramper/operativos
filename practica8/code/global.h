#define HEADS 16   //superficies
#define SECTORS 11
#define CYLINDERS 100
#define SECSIZE 512
#define SECxBLOCK 4

//extra definitions
#define BLOCKSIZE SECSIZE * SECxBLOCK // 2048
#define INODESIZE sizeof(struct INODE)
#define NINODES SECSIZE / INODESIZE * 8 //TODO esto debe ser una funcion

#define DIRECTPTRxINODE 10
#define PTRxBLOCK BLOCKSIZE / sizeof(unsigned short)
#define NOPENFILES 10

#define ERROR	-1
#define SUCCESS	1

#define YES	1 // @romi was here :)
#define NO	0

#define WHENCE_BEG	0
#define WHENCE_CUR	1
#define WHENCE_END	2

#define MAXLEN 80
#define BUFFERSIZE 512

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
	unsigned short blocks[DIRECTPTRxINODE];
	unsigned short indirect;
	unsigned short indirect2;
};

struct OPENFILES {
	int inuse;
	unsigned short inode;
	int currpos;
	int currbloqueenmemoria;
	char buffer[BLOCKSIZE];
	unsigned short buffindirect[BLOCKSIZE];
};

typedef int VDDIR;

struct vddirent
{
	char *d_name;
};

short secboot_en_memoria = 0;  //bandera de sec boot
struct SECBOOT secBoot;	   // estructura secboot

short inodesmap_en_memoria = 0; // inode map flag
unsigned char iNodesMap[SECSIZE]; //mapa de bits de iNode

short datamap_en_memoria = 0;	//bandera de el mapa de datos
unsigned char dataMap[SECSIZE]; // el mapa de datos

short dirraiz_en_memoria = 0;	//bandera de el mapa de datos
struct INODE dirRaiz[NINODES]; // directorio raiz de inodos

short openfiles_inicializada = 0;	//bandera de openfiles
struct OPENFILES openfiles[NOPENFILES]; // open files
