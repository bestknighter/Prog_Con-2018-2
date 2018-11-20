/****************************************************************************
 *																			*
 * Arquivo EstudoDirigido8.c criado por Gabriel Barbosa (12/0050935)		*
 * Problema da barbearia: Uma barbearia tem N cadeiras de espera e uma		*
 * cadeira pra corte. Se cliente chegar e não tiver lugar pra sentar e		*
 * esperar, ele vai embora. Um cliente só senta na cadeira pra cortar o		*
 * cabelo se ela estiver vazia E o barbeiro tiver chamado o próximo. Se		*
 * não tiver cliente, o barbeiro cochila. O barbeiro só corta o cabelo		*
 * quando o cliente autoriza. O cliente só vai embora quando o barbeiro		*
 * diz que acabou de cortar.												*
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


#define MIN_WAIT  500 // Tempo minimo em ms para surgimento de novo cliente
#define MAX_WAIT 9000 // Tempo maximo em ms para surgimento de novo cliente

#define MID_TC_WAIT 5500 // Tempo medio de corte em ms
#define RNG_TC_WAIT  500 // Variacao maxima de tempo de corte em ms


#define N_CADEIRAS 10 // Numero de cadeiras para espera

sem_t cadeirasEspera;	// Quantidade de cadeiras de espera livre
sem_t podeSentar;		// Autorizar proximo cliente a sentar na cadeira
sem_t cadeiraCorte;		// Acompanhar ocupacao da cadeira de corte
sem_t podeCortar;		// Autorizar o barbeiro a cortar o cabelo
sem_t cabeloCortado;	// Autorizar cliente a sair da cadeira de corte

void* barbeiro( void* arg );
void* cliente( void* arg );

int main( int argc, char* argv[] ) {
	pthread_t threadBarbeiro;
	pthread_t threadCliente;

	srand( time(NULL) );

	sem_init( &cadeirasEspera, 0, N_CADEIRAS ); // Existem N_CADEIRAS livres
	sem_init( &podeSentar, 0, 0 ); // Ainda nao pode sentar
	sem_init( &cadeiraCorte, 0, 0 ); // Nao tem ninguem sentado pra ter o cabelo cortado
	sem_init( &podeCortar, 0, 0 ); // Ainda nao pode cortar
	sem_init( &cabeloCortado, 0, 0 ); // Cabelo nao foi cortado

	/* Somente um barbeiro */
	int* arg;
	arg = (int*) malloc( sizeof(int) );
	*arg = 0;
	pthread_create( &threadBarbeiro, NULL, barbeiro, arg );

	/* Um cliente criado a cada rand milissegundos */
	for( int i = 1; 1; i++ ) {
		arg = (int*) malloc( sizeof(int) );
		*arg = i;
		SLEEP( RAND_INTERVAL( MIN_WAIT, MAX_WAIT ) );
		
		// Memory leak mas nao tem problema pq elas sempre terminam e
		// nao preciso acessar de novo
		pthread_create( &threadCliente, NULL, cliente, arg );
	}

	pthread_join( threadBarbeiro, NULL );

	return 0;
}


void* barbeiro( void* arg ) {
	int myArg = *((int*)arg);
	free( arg );
	srand( time(NULL) );

	printf( "[BARBEIRO]: Vamos comecar a trabalhar!\n" );

	while( 1 ) {
		// Espera ter alguém pra cortar o cabelo
		{
			int n;
			sem_getvalue( &cadeirasEspera, &n );
			n = N_CADEIRAS-n;
			printf( "\t%d CLIENTES ESPERANDO ATENDIMENTO\n", n );
			printf( "[BARBEIRO]: Proximo!\n" );
			sem_post( &podeSentar ); // Autoriza proximo cliente a sentar
			if( n <= 0 ) printf( "[BARBEIRO]: Ninguem? Vou tirar um cochilo...\n" );
		}
		sem_wait( &podeCortar ); // Espera ser autorizado a/acordado para cortar o cabelo
		printf( "[BARBEIRO]: Pode deixar!\n" );

		// Corta o cabelo
		printf( "... snip snip snip ...\n" );
		int r = RAND_INTERVAL2( MID_TC_WAIT, RNG_TC_WAIT );
		SLEEP( r );

		// Terminou de cortar
		printf( "\t%d minutos depois\n", r/100 );
		printf( "[BARBEIRO]: E ai? Gostou?\n" );
		sem_post( &cabeloCortado ); // Avisa que terminou de cortar o cabelo
		sem_wait( &cadeiraCorte ); // Espera cliente sair da cadeira de corte
	}
	
	pthread_exit(0);
}

void* cliente( void* arg ) {
	int myArg = *((int*)arg);
	free( arg );

	printf( "[CLIENTE %02d]: Vou no seu ze cortar o cabelo hoje.\n", myArg );
	// Tenta sentar nas cadeiras de espera
	if( 0 == sem_trywait( &cadeirasEspera ) ) {
		printf( "[CLIENTE %02d]: Bom dia, seu ze!\n", myArg );
		sem_wait( &podeSentar ); // Espera ser autorizado a sentar na cadeira de corte

		sem_post( &cadeirasEspera ); // Libera cadeira de espera
		printf( "[CLIENTE %02d]: Vou querer o de sempre! Pode cortar ai...\n", myArg );
		sem_post( &podeCortar ); // Acorda/autoriza barbeiro pra cortar

		sem_wait( &cabeloCortado ); // Espera ter o cabelo cortado
		printf( "[CLIENTE %02d]: Gostei! Ate mais, seu ze!\n", myArg );
		sem_post( &cadeiraCorte ); // Libera cadeira do barbeiro
	} else {
		printf( "[CLIENTE %02d]: A barbearia do seu ze esta lotada. Vou em outra.\n", myArg );
	}

	pthread_exit(0);
}
