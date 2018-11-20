#include "passageiro.hpp"
#include "onibus.hpp"
#include "rodoviaria.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "helper.hpp"

#define N_ONIBUS 10
#define N_PASSAGEIROS 100

Onibus onibus[N_ONIBUS];
Passageiro passageiros[N_PASSAGEIROS];

void* printer( void* arg );

int main( int argc, char* argv[] ) {

	srand( time(NULL) );

	pthread_t printr;

	pthread_create( &printr, NULL, printer, NULL );
	
	Rodoviaria::Start();

	for( int i = 0; i < N_ONIBUS; i++ ) {
		Rodoviaria::RegistrarOnibus( onibus+i );
		onibus[i].Start();
	}

	for( int i = 0; i < N_PASSAGEIROS; i++ ) {
		passageiros[i].Start();
		SLEEP( 500 );
	}


	Rodoviaria::Join();
	Rodoviaria::Destroy();

	pthread_cancel( printr );

	return 0;
}

void* printer( void* arg ) {
	while(true) {
		#ifdef _WIN32
			system( "cls" );
			// printf( "\n\n\n\n" );
		#else
			system( "clear" );
		#endif

		Rodoviaria::printInfo();
		printf( "\n" );
		for( int i = 0; i < N_ONIBUS; i++ ) {
			onibus[i].printInfo();
		}

		printf("\n\n");
		SLEEP( 500 );
	}
}
