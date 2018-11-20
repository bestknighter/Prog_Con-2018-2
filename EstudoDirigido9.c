/****************************************************************************
 *																			*
 * Arquivo EstudoDirigido9.c criado por Gabriel Barbosa (12/0050935)		*
 * Somar todos os elementos de uma matrix usando threads e barreira			*
 * Codigo multiplataforma													*
 * Usar flags -std=c11 -lpthread para compilar para Windows					*
 * Usar flags -std=c11 -pthread para compilar para Linux					*
 *																			*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_barrier_t barrier;
int count = 0;

void* adder( void* arg );

// Os argumentos que vou mandar para dentro de cada thread
typedef struct thread_args_t {
	int ID;
	int* vector;
	int vecSize;
	int* sumResult;
	int sumResultSize;
} thread_args_t;

int main( int argc, char* argv[] ) {

	/* Pegando matrix e seu tamanho do usuario */
	int matrixRows, matrixColumns;
	int *matrix, *sumResult;

	// Pega dimensoes
	printf( "Quantas linhas tem a matrix? " );
	scanf( "%d", &matrixRows );
	printf( "Quantas colunas tem a matrix? " );
	scanf( "%d", &matrixColumns );

	// Aloca matrix e vetor resultado
	matrix = (int*)malloc( matrixRows*matrixColumns*sizeof(int) );
	sumResult = (int*)malloc( matrixRows*sizeof(int) );

	// Pega elementos da matrix
	for( int i = 0; i < matrixRows; i++ ) {
		for( int j = 0; j < matrixColumns; j++ ) {
			printf( "Qual elemento na posicao [%d,%d]? ", i, j );
			scanf( "%d", matrix+matrixColumns*i+j );
		}
	}
	// Imprime matrix para verificacao
	printf( "\nSomando todos os elementos da matrix:\n" );
	for( int i = 0; i < matrixRows; i++ ) {
		for( int j = 0; j < matrixColumns; j++ ) {
			printf( "%2d\t", matrix[matrixColumns*i+j] );
		}
		printf( "\n" );
	}
	printf( "\n" );


	/* Cria threads e realiza a computacao */
	pthread_t* threads;
	thread_args_t* arg;

	// Uma thread para cada linha na matrix
	threads = (pthread_t*)malloc( matrixRows*sizeof(pthread_t) );
	pthread_barrier_init( &barrier, NULL, matrixRows );

	for( int i = 0; i < matrixRows; i++ ) {
		// Preenche argumentos pra thread
		arg = (thread_args_t*) malloc( sizeof(thread_args_t) );
		arg->ID = i;
		arg->vector = matrix+matrixColumns*i;
		arg->vecSize = matrixColumns;
		arg->sumResult = sumResult;
		arg->sumResultSize = matrixRows;

		// Cria thread
		pthread_create( threads+i, NULL, adder, arg );
	}

	// Espera todas acabarem
	for( int i = 0; i < matrixRows; i++ ) {
		pthread_join( threads[i], NULL );
	}

	// Libera recursos
	free( sumResult );
	free( matrix );

	return 0;
}


void* adder( void* arg ) {
	// Para nao ter que ficar fazendo typecasting toda hora
	// Nao criei uma copia para evitar o overhead de copiar tanta memoria
	// mas nao sei se compensa, devido a ter muitos acessos a memoria de outra thread
	thread_args_t* myArg = (thread_args_t*)arg;
	
	printf( "Thread #%02d criada! Comecando a somar...\n", myArg->ID );
	{
		int sum = 0;
		for( int i = 0; i < myArg->vecSize; i++ ) {
			sum += myArg->vector[i];
		}
		// Coloca resultado na posicao correta do vetor
		*(myArg->sumResult+myArg->ID) = sum;
		printf( "Soma da linha %d = %d\n", myArg->ID, sum );
	}

	if( PTHREAD_BARRIER_SERIAL_THREAD == pthread_barrier_wait( &barrier ) ) {
		int sum = 0;
		for( int i = 0; i < myArg->sumResultSize; i++ ) {
			sum += myArg->sumResult[i];
		}
		printf( "Soma total da matrix (feita pela thread #%02d) = %d\n", myArg->ID, sum );
	}

	// Libera argumento
	free( arg );
	pthread_exit(0);
}
