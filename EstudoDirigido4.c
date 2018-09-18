/****************************************************************************
 *																			*
 * Arquivo EstudoDirigido4.c criado por Gabriel Barbosa (12/0050935)		*
 * Vários canibais para somente um cozinheiro								*
 * Codigo multiplataforma													*
 * Usar -std=c11 para compilar												*
 *																			*
****************************************************************************/

#ifdef _WIN32
	#include <windows.h>
	#define SLEEP(ms) Sleep( ms )
#else
	#define _BSD_SOURCE
	#include <unistd.h>
	#define SLEEP(ms) usleep( 1000*(ms) )
#endif

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define PORCOES 25
#define NCANIBS 20

#define COZINHAR_MIN 5000 // ms
#define COZINHAR_ADD 5000 // ms
#define COMER_MIN 2500 // ms
#define COMER_ADD 12500 // ms

pthread_mutex_t l_nc = PTHREAD_MUTEX_INITIALIZER; // Somente para travar o acesso à variável nc
pthread_mutex_t l = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t acabouComida = PTHREAD_COND_INITIALIZER;
pthread_cond_t terminouCozinhar = PTHREAD_COND_INITIALIZER;

int comida = 0;
int nc = 0; // Somente para impressão de quantos canibais estão na fila para pegar comida

void* canibal( void* arg ){
	int seed = *((int*) arg);
	free( arg );
	srand( seed );
	
	while(1) {
		
		// Canibal quer comer
		pthread_mutex_lock( &l_nc );
		++nc;
		if( 0 == comida) printf( "Canibal quer comer! Tem %d canibais na fila\n", nc );
		pthread_mutex_unlock( &l_nc );
		
		// Canibal tenta comer
		pthread_mutex_lock( &l );
		while( 0 == comida ) pthread_cond_wait( &terminouCozinhar, &l ); // Espera ter comida
		if( 0 == --comida ) pthread_cond_signal( &acabouComida ); // Pega comida
		
		// Canibal pegou a comida
		pthread_mutex_lock( &l_nc );
		printf( "Canibal comendo! Restam %d porcoes na mesa e tem %d canibais na fila\n", comida, --nc );
		pthread_mutex_unlock( &l_nc );
		
		pthread_mutex_unlock( &l );
		SLEEP( rand()%COMER_ADD + COMER_MIN ); // Come
	}
	pthread_exit(0);
}

void* cozinheiro( void* arg ){
	int seed = *((int*) arg);
	free( arg );
	srand( seed );
	
	while(1) {

		pthread_mutex_lock( &l );
		while( 0 != comida ) pthread_cond_wait( &acabouComida, &l ); // Conzinheiro espera canibais avisarem que acabou a comida

		printf( "Cozinheiro preparando comida\n" );
		SLEEP( rand()%COZINHAR_ADD + COZINHAR_MIN ); // Cozinha
		
		// Coloca comida na mesa
		comida += PORCOES;
		printf( "Cozinheiro colocando %d porcoes de comida na mesa\n", PORCOES );
		pthread_cond_broadcast( &terminouCozinhar ); // Avisa que tem comida
		pthread_mutex_unlock( &l );
	}
	pthread_exit(0);
}

int main( int argc, char* argv[] ) {
	pthread_t thread;
	
	srand( time(NULL) );
	int* seed;
	
	// Cria cozinheiro
	seed = (int*) malloc( sizeof(int) );
	*seed = rand();
	pthread_create( &thread, NULL, cozinheiro, seed );
	
	// Cria canibais
	for( int i = 0; i < NCANIBS; i++ ) {
		seed = (int*) malloc( sizeof(int) );
		*seed = rand();
		pthread_create( &thread, NULL, canibal, seed );
	}
	
	pthread_join( thread, NULL ); // Espera encerrar
	
	return 0;
}
