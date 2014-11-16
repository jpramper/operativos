// Definición de constantes

#define NINGUNO -1

#define VERSION "mmu versión 10.0326.00\n"

// tamanio de una pagina (4kb)
#define PAGESIZE 4096 
// tamanio de la memoria completa (32kb)
#define PHISICALMEMORYSIZE 32*1024 
// tamanio de la tabla fisica (8)
#define SYSTEMFRAMETABLESIZE PHISICALMEMORYSIZE/PAGESIZE 
// total de marcos (8)
#define TOTFRAMES SYSTEMFRAMETABLESIZE 
// numero de procesos que corre el programa
#define MAXPROC 4 
// tamanio de la tabla de paginas de un proceso (4)
#define PROCESSPAGETABLESIZE 2*SYSTEMFRAMETABLESIZE/MAXPROC  
// tamaño en memoria de la talba de marcos
#define TABLESSIZE 2*SYSTEMFRAMETABLESIZE*sizeof(struct SYSTEMFRAMETABLE) 

// Definición de estructuras

struct SYSTEMFRAMETABLE {
    int assigned;

    char *paddress; // No modificar (apuntador a pagina del proceso)
    int shmidframe;  // No modificar
};

struct PROCESSPAGETABLE {
    int presente;
    int modificado;
    int framenumber;
    unsigned long tarrived;
    unsigned long tlastaccess;

    int attached;   // No modificar
};


