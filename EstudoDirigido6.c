/****************************************************************************
 *																			*
 * Arquivo EstudoDirigido6.c criado por Gabriel Barbosa (12/0050935)		*
 * Um estacionamento de uma instituição de ensino tem estacionamento		*
 * prioritário para professores, em seguida para funcionários, e por		*
 * último, para os alunos.													*
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
#include <time.h>

#define PROF_ESTAC 10000 // Tempo de estacionamento em ms
#define FUNC_ESTAC 7500 // Tempo de estacionamento em ms
#define ALUN_ESTAC 4000 // Tempo de estacionamento em ms
#define RANG_ESTAC 3000 // Variacao de tempo possivel em ms

#define PROF_FORA 5000 // Tempo fora do estacionamento em ms
#define FUNC_FORA 8000 // Tempo fora do estacionamento em ms
#define ALUN_FORA 10000 // Tempo fora do estacionamento em ms
#define RANG_FORA 4000 // Variacao de tempo possivel em ms

#define N_PROFS 10
#define N_FUNCS 20
#define N_ALUNS 40

#define N_VAGAS 30

#define PRINT_INFO() {printf( "Estacionamento tem %2d vagas livres; Na fila temos %2d professores, %2d funcionarios e %2d alunos\n", N_VAGAS-vagasOcupadas, nProfsNaFila, nFuncsNaFila, nAlunosNaFila );}

void* professor( void* arg );
void* funcionario( void* arg );
void* aluno( void* arg );

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int vagasOcupadas = 0;
pthread_cond_t liberouVaga = PTHREAD_COND_INITIALIZER;
int nProfsNaFila = 0;
int nFuncsNaFila = 0;
int nAlunosNaFila = 0;

int main( int argc, char* argv[] ) {
	pthread_t thread;

	srand( time(NULL) );

	int* arg;
	// Cria professores
	for( int i = 0; i < N_PROFS; i++ ) {
		arg = (int*) malloc( sizeof(int) );
		*arg = rand();
		pthread_create( &thread, NULL, professor, arg );
	}

	// Cria funcionarios
	for( int i = 0; i < N_FUNCS; i++ ) {
		arg = (int*) malloc( sizeof(int) );
		*arg = rand();
		pthread_create( &thread, NULL, funcionario, arg );
	}

	// Cria alunos
	for( int i = 0; i < N_ALUNS; i++ ) {
		arg = (int*) malloc( sizeof(int) );
		*arg = rand();
		pthread_create( &thread, NULL, aluno, arg );
	}

	pthread_join( thread, NULL );

	return 0;
}


void* professor( void* arg ) {
	int myArg = *((int*)arg);
	free( arg );

	srand( myArg );

	while(1) {
		// Chega no estacionamento
		pthread_mutex_lock( &mutex );
			++nProfsNaFila;
			printf( "Professor quer estacionar\t\t" );
			PRINT_INFO();
			// Espera ter vagas
			while( N_VAGAS == vagasOcupadas ) pthread_cond_wait( &liberouVaga, &mutex );

			// Tem vaga
			++vagasOcupadas;
			--nProfsNaFila;
			printf( "Professor conseguiu estacionar\t\t" );
			PRINT_INFO();
		pthread_mutex_unlock( &mutex );

		// Faz o que tem que fazer
		SLEEP( RAND_INTERVAL2( PROF_ESTAC, RANG_ESTAC ) );

		// Sai do estacionamento
		pthread_mutex_lock( &mutex );
			--vagasOcupadas;
			pthread_cond_broadcast( &liberouVaga );
			printf( "Professor saiu do estacionamento\t" );
			PRINT_INFO();
		pthread_mutex_unlock( &mutex );

		// Fica em casa
		SLEEP( RAND_INTERVAL2( PROF_FORA, RANG_FORA ) );
	}

	pthread_exit(0);
}

void* funcionario( void* arg ) {
	int myArg = *((int*)arg);
	free( arg );

	srand( myArg );

	while(1) {
		// Chega no estacionamento
		pthread_mutex_lock( &mutex );
			++nFuncsNaFila;
			printf( "Funcionario quer estacionar\t\t" );
			PRINT_INFO();
			// Espera ter vagas e nenhum professor na fila
			while( N_VAGAS == vagasOcupadas || 0 < nProfsNaFila ) pthread_cond_wait( &liberouVaga, &mutex );

			// Tem vaga e nao tem professor na fila
			++vagasOcupadas;
			--nFuncsNaFila;
			printf( "Funcionario conseguiu estacionar\t" );
			PRINT_INFO();
		pthread_mutex_unlock( &mutex );

		// Faz o que tem que fazer
		SLEEP( RAND_INTERVAL2( FUNC_ESTAC, RANG_ESTAC ) );

		// Sai do estacionamento
		pthread_mutex_lock( &mutex );
			--vagasOcupadas;
			pthread_cond_broadcast( &liberouVaga );
			printf( "Funcionario saiu do estacionamento\t" );
			PRINT_INFO();
		pthread_mutex_unlock( &mutex );

		// Fica em casa
		SLEEP( RAND_INTERVAL2( FUNC_FORA, RANG_FORA ) );
	}


	pthread_exit(0);
}

void* aluno( void* arg ) {
	int myArg = *((int*)arg);
	free( arg );

	srand( myArg );

	while(1) {
		// Chega no estacionamento
		pthread_mutex_lock( &mutex );
			++nAlunosNaFila;
			printf( "Aluno quer estacionar\t\t\t" );
			PRINT_INFO();
			// Espera ter vaga e nao ter nem professor nem funcionario na fila
			while( N_VAGAS == vagasOcupadas || 0 < nProfsNaFila || 0 < nFuncsNaFila ) pthread_cond_wait( &liberouVaga, &mutex );

			// Tem vaga, nao tem professor e nem funcionario na fila
			++vagasOcupadas;
			--nAlunosNaFila;
			printf( "Aluno conseguiu estacionar\t\t" );
			PRINT_INFO();
		pthread_mutex_unlock( &mutex );

		// Faz o que tem que fazer
		SLEEP( RAND_INTERVAL2( ALUN_ESTAC, RANG_ESTAC ) );

		// Sai do estacionamento
		pthread_mutex_lock( &mutex );
			--vagasOcupadas;
			pthread_cond_broadcast( &liberouVaga );
			printf( "Aluno saiu do estacionamento\t\t" );
			PRINT_INFO();
		pthread_mutex_unlock( &mutex );

		// Fica em casa
		SLEEP( RAND_INTERVAL2( ALUN_FORA, RANG_FORA ) );
	}

	pthread_exit(0);
}
