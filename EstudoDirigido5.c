/****************************************************************************
 *																			*
 * Arquivo EstudoDirigido5.c criado por Gabriel Barbosa (12/0050935)		*
 * Problema dos produtores e consumidores									*
 * Codigo multiplataforma													*
 * Usar flags -std=c11 -lpthread para compilar para windows					*
 * Usar flags -std=c11 -pthread para compilar para linux					*
 *																			*
****************************************************************************/

#ifdef _WIN32
	#include <windows.h>
	
	#define SLEEP(ms) Sleep(ms)
#else
	#define _BSD_SOURCE
	#include <unistd.h>

	#define SLEEP(ms) usleep( 1000*(ms) )
#endif

#define SLEEP_RINTERVAL(min, max) SLEEP( rand()%(max - min)+min )

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 10 // Tamanho do buffer
int count = 0; // Buffer

#define N_PROD 50 // Quantidade de produtores
#define N_CONS 50 // Quantidade de consumidores

#define PROD_MIN_WAIT 3750 // Tempo minimo de producao de item, em ms
#define PROD_MAX_WAIT 6250 // Tempo maximo de producao de item, em ms
#define CONS_MIN_WAIT 4000 // Tempo minimo de consumo de item, em ms
#define CONS_MAX_WAIT 6000 // Tempo maximo de consumo de item, em ms

pthread_cond_t buffer_tem_espaco = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_tem_item = PTHREAD_COND_INITIALIZER;
pthread_mutex_t l_buffer = PTHREAD_MUTEX_INITIALIZER;

void* producer( void* arg );
void* consumer( void* arg );

int main( int argc, char* argv[] ) {
	pthread_t thread;

	printf( "Criando %d produtores e %d consumidores e buffer de tamanho %d.\n", N_PROD, N_CONS, N );
	printf( "Produtores levam de %0.2fs a %0.2fs para produzir um item.\n", PROD_MIN_WAIT/1000.0, PROD_MAX_WAIT/1000.0 );
	printf( "Consumidores levam de %0.2fs a %0.2fs para consumir um item.\n", CONS_MIN_WAIT/1000.0, CONS_MAX_WAIT/1000.0 );

	srand( time(NULL) );

	int* seed;
	// Cria produtores
	for( int i = 0; i < N_PROD; i++ ) {
		seed = (int*) malloc( sizeof(int) );
		*seed = rand();
		pthread_create( &thread, NULL, producer, seed );
	}

	// Cria consumidores
	for( int i = 0; i < N_CONS; i++ ) {
		seed = (int*) malloc( sizeof(int) );
		*seed = rand();
		pthread_create( &thread, NULL, consumer, seed );
	}

	pthread_join( thread, NULL );

	return 0;
}

void* producer( void* arg ) {
	int seed = *((int*)arg);
	free(arg);
	srand( seed );

	int item;

	while(1) {
		// Produz item
		SLEEP_RINTERVAL( PROD_MIN_WAIT, PROD_MAX_WAIT );

		// Da pra adicionar item ao buffer?
		pthread_mutex_lock( &l_buffer );
		while( N == count ) pthread_cond_wait( &buffer_tem_espaco, &l_buffer );
		
		// Agora da!
		printf( "Item colocado! Existem %d itens no buffer. ", ++count );
		if( 1 == count ) {
			// Se buffer nao esta mais vazio, acorde os consumidores
			printf( "Acordando consumidores." );
			pthread_cond_broadcast( &buffer_tem_item );
		}
		printf( "\n" );
		pthread_mutex_unlock( &l_buffer );
	}

	pthread_exit(0);
}

void* consumer( void* arg ) {
	int seed = *((int*)arg);
	free(arg);
	srand( seed );

	while(1) {
		// Da para pegar item?
		pthread_mutex_lock( &l_buffer );
		while( 0 == count ) pthread_cond_wait( &buffer_tem_item, &l_buffer );

		// Agora da!
		printf( "Item removido! Existem %d itens no buffer. ", --count );
		if( N-1 == count ) {
			// Se buffer nao esta mais lotado, acorda os produtores
			printf( "Acordando produtores." );
			pthread_cond_broadcast( &buffer_tem_espaco );
		}
		pthread_mutex_unlock( &l_buffer );
		printf( "\n" );

		// Consome item
		SLEEP_RINTERVAL( CONS_MIN_WAIT, CONS_MAX_WAIT );
	}

	pthread_exit(0);
}