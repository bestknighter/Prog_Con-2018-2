#include "passageiro.hpp"

#include "helper.hpp"

#include "onibus.hpp"
#include "rodoviaria.hpp"

int Passageiro::nextID = 0;

void Passageiro::Start() {
	if( 0 == thread ) {
		pthread_create( &(this->thread), NULL, this->main, this);
	}
}

Passageiro::Passageiro() : myID( nextID++ ), onibus( nullptr ) {}

Passageiro::~Passageiro() {
	if( 0 != thread ) {
		pthread_detach( this->thread );
		pthread_cancel( this->thread );
	}
}

void Passageiro::Join() {
	if( 0 != thread ) {
		pthread_join( this->thread, NULL);
	}
}

const std::vector< Onibus* > Passageiro::GetPreferencias() {
	std::vector< Onibus* >& listaOnibus = Rodoviaria::GetListaOnibus();
	std::vector< Onibus* > preferenciasOrdenadas;
	int tam = listaOnibus.size();
	for( int i = 0; i < tam; i++ ) {
		Onibus* o = listaOnibus[i];
		if( 0 == i%this->tolerancia ) {
			preferenciasOrdenadas.push_back( o );
		}
	}

	std::vector< Onibus* > preferencias;
	for( int i = preferenciasOrdenadas.size(); i > 0; i-- ) {
		int index = rand()%i;
		preferencias.push_back( preferenciasOrdenadas[index] );
		preferenciasOrdenadas.erase( preferenciasOrdenadas.begin() + index );
	}
	return preferencias;
}

// -----------------------------------------------------------------------------------------------------------

void* Passageiro::main( void* arg ) {
	Passageiro* ref = (Passageiro*)arg;

	srand( time(NULL) );
	while(true) {
		ref->tolerancia = 1 + (rand() % (MAX_TOL-1));
		do{
			std::vector< Onibus* > onibus = ref->GetPreferencias();
			for( int i = 0; i < (int)onibus.size(); i++ ) {
				// So tenta entrar se o onibus ja conseguiu estacionar
				if( onibus[i]->isEstacionado() && onibus[i]->tentaEntrar() ) {
					ref->onibus = onibus[i];
					break;
				}
			}
			if( nullptr == ref->onibus ) {
				// Se nao conseguiu nenhum, espera atualizar e repete
				Rodoviaria::esperarAtualizacao_Passageiro();
			}
		} while( nullptr == ref->onibus );

		ref->onibus->sentar();
		ref->onibus = nullptr;
		SLEEP( 1000 );
	}

	pthread_exit( 0 );
}
