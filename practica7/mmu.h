// Definición de constantes
#define NINGUNO -1
#define ERROR -1
#define SUCCESS  1
#define TRUE  1
#define FALSE 0
#define VERSION "mmu versión 10.0326.00\n"

//frames
#define NFRAMES     8
#define FRAMESIZE   (4 * 1024)
//pages
#define NPAGES      16
#define PAGESIZE    FRAMESIZE
//memories
#define MEMSIZE     (NFRAMES * FRAMESIZE)
#define SWAPSIZE    (NPAGES * PAGESIZE)
//process
#define NPROC 4
#define PAGESPERPROC NPAGES/NPROC
#define SHAREDTABLESIZE NPAGES*sizeof(struct FRAMETABLE)

// Definición de estructuras

struct FRAMETABLE {
    int assigned;

    char *paddress; // No modificar (apuntador a pagina del proceso)
    int shmidframe;  // No modificar
};

struct PAGETABLE {
    int presente;
    int modificado;
    int framenumber;
    unsigned long tarrived;
    unsigned long tlastaccess;

    int attached;   // No modificar
};
