/****************************************************************************
 *																			*
 * Arquivo EstudoDirigido7.c criado por Gabriel Barbosa (12/0050935)		*
 * Problema do produtor/consumidor usando semáforos							*
 * Codigo multiplataforma													*
 * Usar flags -std=c11 -lpthread para compilar para Windows					*
 * Usar flags -std=c11 -pthread para compilar para Linux					*
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

#define RAND_INTERVAL(min, max) ( rand()%( (max) - (min) ) + (min) )
#define RAND_INTERVAL2(mid, rng) ( rand()%( 2 * (rng) ) + (mid) - (rng) )

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


#define MIN_WAIT_P 2000 // Tempo minimo em ms
#define MAX_WAIT_P 4000 // Tempo maximo em ms

#define MIN_WAIT_C 4000 // Tempo minimo em ms
#define MAX_WAIT_C 6000 // Tempo maximo em ms

#define PRODUTORES 2
#define CONSUMIDORES 3

#define BUFFER_SIZE 5

// Mutex e count são usados somente para se imprimir quantos itens tem dentro do buffer
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int count = 0;

sem_t vazio;
sem_t cheio;

void* produtores( void* arg );
void* consumidores( void* arg );

int main( int argc, char* argv[] ) {
	pthread_t thread[PRODUTORES + CONSUMIDORES];

	// Inicializando semáforos
	sem_init( &vazio, 0, BUFFER_SIZE );
	sem_init( &cheio, 0, 0 );

	srand( time(NULL) );

	printf( "Criando %d produtores e %d consumidores\n", PRODUTORES, CONSUMIDORES );
	printf( "Haverao %d espacos no buffer.\n\n", BUFFER_SIZE );

	int* arg;
	for( int i = 0; i < PRODUTORES; i++ ) {
		arg = (int*) malloc( sizeof(int) );
		*arg = rand();
		pthread_create( thread+i, NULL, produtores, arg );
	}
	for( int i = 0; i < CONSUMIDORES; i++ ) {
		arg = (int*) malloc( sizeof(int) );
		*arg = rand();
		pthread_create( thread+i+PRODUTORES, NULL, consumidores, arg );
	}

	pthread_join( thread[0], NULL );

	return 0;
}


void* produtores( void* arg ) {
	int myArg = *((int*)arg);
	free( arg );

	srand( myArg );
	
	while(1) {
		// Produz item
		printf( "Produtor ID #%d produzindo item.\n", myArg );
		SLEEP( RAND_INTERVAL( MIN_WAIT_P, MAX_WAIT_P ) );

		// Coloca item
		sem_wait( &vazio );
		sem_post( &cheio );
		pthread_mutex_lock( &mutex );
		count++;
		printf( "Produtor ID #%d adicionou item no buffer.\t%d/%d itens no buffer\n", myArg, count, BUFFER_SIZE );
		pthread_mutex_unlock( &mutex );
	}

	pthread_exit(0);
}


void* consumidores( void* arg ) {
	int myArg = *((int*)arg);
	free( arg );

	srand( myArg );
	
	while(1) {
		// Pega item
		sem_wait( &cheio );
		sem_post( &vazio );
		pthread_mutex_lock( &mutex );
		count--;
		printf( "Consumidor ID #%d pegou item do buffer.\t%d/%d itens no buffer\n", myArg, count, BUFFER_SIZE );
		pthread_mutex_unlock( &mutex );

		// Consome item
		SLEEP( RAND_INTERVAL( MIN_WAIT_C, MAX_WAIT_C ) );
		printf( "Consumidor ID #%d consumiu item.\n", myArg );
	}

	pthread_exit(0);
}
