#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_FILAS 8
#define TAM_LINEA 16

//DEFINO LA ESTRUCTURA CACHE

typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

int globaltime = 0;
int numfallos = 0;
int numAccesos = 0; 

 //SE ESTABLECE LOS VALORES PREDETERMINADOS para las etiquetes

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]) {
    for (int i = 0; i < NUM_FILAS; ++i) {
        tbl[i].ETQ = 0xFF;
        for (int j = 0; j < TAM_LINEA; ++j) {
            tbl[i].Data[j] = 0x23F;
        }
    }
}
 //IMPRIME EL CONTENIDO 

void VolcarCACHE(T_CACHE_LINE *tbl) {
    printf("\nContenido de la caché:\n");
    for (int i = 0; i < NUM_FILAS; ++i) {
        printf("Linea %d: ETQ: %X, Datos: ", i, tbl[i].ETQ);
        for (int j = 0; j < TAM_LINEA; ++j) {
            printf("%02X ", tbl[i].Data[j]);
        }
        printf("\n");
    }
}

//DIVIDE LOS CAMPOS CUANDO LE DAS LA DIRECCION DE MEMORIA 

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque) {
    *palabra = addr & 0xF;
    *linea = (addr >> 4) & 0x7;
    *bloque = (addr >> 7) & 0x1F;
    *ETQ = (addr >> 12) & 0x1F;
}

//ESTA FUNCION SIRVE PARA TRATAR LOS FALLOS , CADA VEZ QUE HAY UN FALLO LA LLAMAN

void TratarFallo(T_CACHE_LINE *tbl, unsigned char *MRAM, int ETQ, int linea, int bloque) {
    printf("T:%d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X\n",
           globaltime, numfallos, linea * 16 + bloque * 128, ETQ, linea, bloque, bloque * 2);

    
    globaltime += 10;

   
    int inicio_bloque = bloque * 128;
    for (int i = 0; i < TAM_LINEA; ++i) {
        tbl[linea].Data[i] = MRAM[inicio_bloque + i];
    }

   
    tbl[linea].ETQ = ETQ;
}

//CON LA FUNCION MAIN LLAMAMOS A SIMUL_RAM la simulacion de la memoria principal y el texto un array para almacenar el texto leido

int main() {
    T_CACHE_LINE tbl[NUM_FILAS];
    unsigned char Simul_RAM[4096];
    char texto[100];
    int direccion;

//LLAMAMOS A LIMPIAR CACHE para inicializarlo y cargar el contenido de la memoria
   
    LimpiarCACHE(tbl);

    FILE *ram_file = fopen("CONTENTS_RAM.bin", "rb");
    if (!ram_file) {
        printf("Error: No se pudo abrir el archivo CONTENTS_RAM.bin\n");
        return -1;
    }
    fread(Simul_RAM, sizeof(unsigned char), sizeof(Simul_RAM), ram_file);
    fclose(ram_file);

    // IMPRIMIMOS EL ESTADO DE LA CACHE UNA VEZ QUE LO HEMOS INICIALIZADO
    VolcarCACHE(tbl);

  //abrimos las direcciones de memoria
  
    FILE *dirs_file = fopen("dirs_memoria.txt", "r");
    if (!dirs_file) {
        printf("Error: No se pudo abrir el archivo dirs_memoria.txt\n");
        return -1;
    }

//POR CADA ACCESO VERIFICA SI HAY UN ACIERTO EN LA CACHE , SI NO LO HAY LLAMA A TRATAR FALLOS e INCREMENTA EL CONTADOR DE NUMEROS DE FALLOS

  
    while (fscanf(dirs_file, "%x", &direccion) == 1) {
        int ETQ, palabra, linea, bloque;
        ParsearDireccion(direccion, &ETQ, &palabra, &linea, &bloque);

       
        int acierto = 0;
        if (tbl[linea].ETQ == ETQ) {
            acierto = 1;
        }

        if (acierto) {
            // Manejar el acierto de caché
            printf("T:%d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X\n",
                   globaltime, direccion, ETQ, linea, palabra, tbl[linea].Data[palabra]);
        
        } else {
          
            TratarFallo(tbl, Simul_RAM, ETQ, linea, bloque);
            numfallos++;
        }

      
        numAccesos++;

      
        VolcarCACHE(tbl);

     
        sleep(1);
    }

    fclose(dirs_file);


//UNA VEZ QUE ACABA EL PROCESO IMPRIME POR PANTALLA, EL NUMERO DE FALLOS Y TIEMPO MEDIO
  
    printf("\nNúmero total de accesos: %d\n", numAccesos);
    printf("Número de fallos: %d\n", numfallos);
    float tiempoMedioAcceso = (float)globaltime / numAccesos;
    printf("Tiempo medio de acceso: %f segundos\n", tiempoMedioAcceso);

// Y POR ULTIMO GUARDA EL CONTENIDO EL CONTEST_CACHE
  
    FILE *cache_file = fopen("CONTENTS_CACHE.bin", "wb");
    if (!cache_file) {
        printf("Error: No se pudo abrir el archivo CONTENTS_CACHE.bin\n");
        return -1;
    }
    fwrite(tbl, sizeof(T_CACHE_LINE), NUM_FILAS, cache_file);
    fclose(cache_file);

 
    printf("\nTexto leído desde la caché:\n");
    for (int i = 0; i < NUM_FILAS; ++i) {
        for (int j = 0; j < TAM_LINEA; ++j) {
            printf("%c", tbl[i].Data[j]);
        }
    }
    printf("\n");

    return 0;
}
