#include "file_cleaner.h"

void vaciarArchivo(char* ruta){
	FILE* fclean = fopen(ruta, "w");
	fclose(fclean);
}
