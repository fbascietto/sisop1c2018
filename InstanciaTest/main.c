/*
 * main.c
 *
 *  Created on: May 1, 2018
 *      Author: utnso
 */


#include <CUnit/Basic.h>
#include <unistd.h>
#include "../Instancia/funcionesInstancia.c"
#include <commons/collections/list.h>



int inicializar() {
	entradas = list_create();
	return 1;
}

void test_crea_correctamente_las_entradas_y_se_agregan_a_la_lista(){

	almacenarEntrada("entrada1","de pendejo te sigo");
	almacenarEntrada("entrada2","junto a racing siempre a todos lados");
	almacenarEntrada("entrada3","nos bancamos la quiebra");
	almacenarEntrada("entrada3","el descenso y fuimos alguilados");

}


void agregar_test(){

}

int main() {
   CU_initialize_registry();

   agregar_test();

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   return CU_get_error();
}
