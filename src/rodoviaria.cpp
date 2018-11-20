#include "rodoviaria.hpp"

#include "onibus.hpp"

#include "helper.hpp"

Rodoviaria* Rodoviaria::instance = nullptr;

void Rodoviaria::Start() {
	if( nullptr == instance ) {
		instance = new Rodoviaria();
		pthread_create( &(instance->thread), NULL, instance->main, instance);
	}
}

void Rodoviaria::Destroy() {
	if( nullptr != instance) delete instance;
}

Rodoviaria::Rodoviaria() {
	this->atualizou = false;
	this->l_atualizou = PTHREAD_MUTEX_INITIALIZER;
	this->acorda = PTHREAD_COND_INITIALIZER;

	this->l_novosDados = PTHREAD_MUTEX_INITIALIZER;
	this->tentarDeNovo = PTHREAD_COND_INITIALIZER;

	sem_init( &(this->s_vagas), 0, N_VAGAS );

	this->l_estacionamento = PTHREAD_MUTEX_INITIALIZER;
}

Rodoviaria::~Rodoviaria() {
	if( 0 != thread ) {
		pthread_detach( this->thread );
		pthread_cancel( this->thread );
	}
}

void Rodoviaria::Join() {
	if( 0 != instance->thread ) {
		pthread_join( instance->thread, NULL);
	}
}

void Rodoviaria::WakeUp() {
	pthread_mutex_lock( &(instance->l_atualizou) );
	instance->atualizou = true;
	pthread_mutex_unlock( &(instance->l_atualizou) );
	pthread_cond_signal( &(instance->acorda) );
}

void Rodoviaria::entrarNaFila_Onibus( Onibus* onibus ) {
	sem_wait( &(instance->s_vagas) );
	RAND_INTERVAL( 1000, 5000 );
	onibus->SetEstacionado( true );
	WakeUp();
}

void Rodoviaria::sairDaRodoviaria_Onibus( Onibus* onibus ) {
	onibus->SetEstacionado( false );
	sem_post( &(instance->s_vagas) );
	WakeUp();
}

void Rodoviaria::RegistrarOnibus( Onibus* onibus ) {
	pthread_mutex_lock( &(instance->l_estacionamento) );
	instance->onibusRegistrados.push_back( onibus );
	pthread_mutex_unlock( &(instance->l_estacionamento) );
}

void Rodoviaria::esperarAtualizacao_Passageiro() {
	pthread_mutex_lock( &(instance->l_novosDados) );
	pthread_cond_wait( &(instance->tentarDeNovo), &(instance->l_novosDados) );
	pthread_mutex_unlock( &(instance->l_novosDados) );
	RAND_INTERVAL( 1000, 5000 );
}

std::vector< Onibus* >& Rodoviaria::GetListaOnibus() {
	return instance->onibusRegistrados;
}

void Rodoviaria::printInfo() {
	int vagas;
	sem_getvalue( &(instance->s_vagas), &vagas );
	vagas = vagas < 0 ? 0 : vagas;
	printf( "\t\t\t\tRODOVIARIA LOCAL DA RUA DO BAIRRO\n");
	printf( "%d (de %d) vagas de onibus livres\n", vagas, N_VAGAS );
}

// -----------------------------------------------------------------------------------------------------------

void* Rodoviaria::main( void* arg ) {
	Rodoviaria* ref = (Rodoviaria*)arg;

	srand( time(NULL) );

	while(1) {
		// Espera ter alguma novidade
		pthread_mutex_lock( &(ref->l_atualizou) );
		while( !ref->atualizou ) pthread_cond_wait( &(ref->acorda), &(ref->l_atualizou) );
		ref->atualizou = false;
		pthread_mutex_unlock( &(ref->l_atualizou) );

		// Avisa passageiros que tem novidade
		pthread_cond_broadcast( &(ref->tentarDeNovo) );
	}

	pthread_exit( 0 );
}
