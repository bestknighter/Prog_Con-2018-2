#include "onibus.hpp"

#include "helper.hpp"
#include "rodoviaria.hpp"

int Onibus::nextID = 0;

void Onibus::Start() {
	if( 0 == thread ) {
		pthread_create( &(this->thread), NULL, this->main, this);
	}
}

Onibus::Onibus() : myID( nextID++ ) {
	this->estacionou = false;
	this->l_estacionou = PTHREAD_MUTEX_INITIALIZER;

	this->podeSair = PTHREAD_COND_INITIALIZER;

	this->l_lotacao = PTHREAD_MUTEX_INITIALIZER;
	sem_init( &(this->s_lotacao), 0, MAX_CAPACITY );

	this->chegouDestino = false;
	this->podeDescer = PTHREAD_COND_INITIALIZER;
	this->l_chegouDestino = PTHREAD_MUTEX_INITIALIZER;
	this->esvaziou = PTHREAD_COND_INITIALIZER;

	this->ETA = 0;
	this->l_ETA = PTHREAD_MUTEX_INITIALIZER;

}

Onibus::~Onibus() {
	if( 0 != thread ) {
		pthread_detach( this->thread );
		pthread_cancel( this->thread );
	}
}

void Onibus::Join() {
	if( 0 != thread ) {
		pthread_join( this->thread, NULL);
	}
}

long long Onibus::GetETA() {
	long long currentETA;
	pthread_mutex_lock( &(this->l_ETA) );
	currentETA = ETA - time(NULL);
	pthread_mutex_unlock( &(this->l_ETA) );
	return currentETA;
}

bool Onibus::isEstacionado() {
	bool estacionou;
	pthread_mutex_lock( &(this->l_estacionou) );
	estacionou = this->estacionou;
	pthread_mutex_unlock( &(this->l_estacionou) );
	return estacionou;
}

void Onibus::SetEstacionado( bool estacionado ) {
	pthread_mutex_lock( &(this->l_estacionou) );
	this->estacionou = estacionado;
	pthread_mutex_unlock( &(this->l_estacionou) );
}

bool Onibus::tentaEntrar() {
	RAND_INTERVAL( 1000, 5000 );
	pthread_mutex_lock( &(this->l_lotacao) );
	bool ret = sem_trywait( &(this->s_lotacao) ) == 0;
	pthread_mutex_unlock( &(this->l_lotacao) );
	return ret;
}

void Onibus::sentar() {
	RAND_INTERVAL( 1000, 5000 );
	pthread_mutex_lock( &(this->l_lotacao) );
	int vagas;
	sem_getvalue( &(this->s_lotacao), &vagas );
	if( vagas <= 0 ) pthread_cond_signal( &(this->podeSair) );
	pthread_mutex_unlock( &(this->l_lotacao) );

	pthread_mutex_lock( &(this->l_chegouDestino) );
	while( !this->chegouDestino ) pthread_cond_wait( &(this->podeDescer), &(this->l_chegouDestino) );
	pthread_mutex_unlock( &(this->l_chegouDestino) );
	RAND_INTERVAL( 1000, 5000 );

	pthread_mutex_lock( &(this->l_lotacao) );
	sem_post( &(this->s_lotacao) );
	sem_getvalue( &(this->s_lotacao), &vagas );
	if( vagas == MAX_CAPACITY ) pthread_cond_signal( &(this->esvaziou) );
	pthread_mutex_unlock( &(this->l_lotacao) );
}

void Onibus::SetETA( long long newETA ) {
	pthread_mutex_lock( &(this->l_ETA) );
	this->ETA = newETA;
	pthread_mutex_unlock( &(this->l_ETA) );
}

void Onibus::printInfo() {
	printf( "[Onibus %02d]: ", myID+1 );
	int vagas;
	sem_getvalue( &s_lotacao, &vagas );
	long long eta = GetETA();
	if( estacionou ) {
		printf( "Estacionado na rodoviaria\t\t\tVAGAS: %2d\n", vagas );
	} else if ( chegouDestino ) {
		printf( "Chegou no destino. Descarregando passageiros.\n" );
	} else if ( eta >= 0 ) {
		if( eta > TEMPO_PERCURSO/1000 ) {
			printf ( "Em transito\t\t\t\tVAGAS: %2d\t\tETA PRO DESTINO (em segundos):\t\t%3lld\n", vagas, eta - TEMPO_PERCURSO/1000 );
		} else {
			printf ( "Em transito\t\t\t\tVAGAS: %2d\t\tETA PRA RODOVIARIA (em segundos):\t%3lld\n", vagas, eta );
		}
	} else {
		printf( "Esperando abrir vaga na rodoviaria\t\tVAGAS: %2d\n", vagas );
	}
}

// -----------------------------------------------------------------------------------------------------------

void* Onibus::main( void* arg ) {
	Onibus* ref = (Onibus*)arg;
	srand( time(NULL) );

	while(1) {
		// Chegando na rodoviaria
		ref->SetETA( time(NULL) );
		Rodoviaria::entrarNaFila_Onibus( ref );

		esperaNaRodoviaria( ref );

		// Faz a viagem
		Rodoviaria::sairDaRodoviaria_Onibus( ref );
		ref->SetETA( time(NULL) + 2*TEMPO_PERCURSO/1000 );
		SLEEP( TEMPO_PERCURSO );

		// Chegou no destino
		chegaDestino( ref );
		esperaEsvaziar( ref );
		RAND_INTERVAL( 1000, 5000 );
		ref->chegouDestino = false;


		// Faz a viagem de volta pra rodoviaria
		ref->SetETA( time(NULL) + TEMPO_PERCURSO/1000 );
		SLEEP( TEMPO_PERCURSO );
	}

	pthread_exit( 0 );
}

// Enquanto nao estiver lotado, nao sai da rodoviaria,
// a nao ser que espere mais de 13 segundos
void Onibus::esperaNaRodoviaria( Onibus* ref ) {
	timespec t = {13 + time(NULL), 0};
	pthread_mutex_lock( &(ref->l_lotacao) );

	int freeSeats;
	sem_getvalue( &(ref->s_lotacao), &freeSeats );
	while( freeSeats > 0 ) {
		int retVal = pthread_cond_timedwait( &(ref->podeSair), &(ref->l_lotacao), &t );
		if( ETIMEDOUT == retVal ) break;
		sem_getvalue( &(ref->s_lotacao), &freeSeats );
	}

	pthread_mutex_unlock( &(ref->l_lotacao) );
}

// Espera todo mundo descer do onibus
void Onibus::esperaEsvaziar( Onibus* ref ) {
	pthread_mutex_lock( &(ref->l_lotacao) );

	int freeSeats;
	sem_getvalue( &(ref->s_lotacao), &freeSeats );
	while( freeSeats < MAX_CAPACITY ) {
		pthread_cond_wait( &(ref->esvaziou), &(ref->l_lotacao) );
		sem_getvalue( &(ref->s_lotacao), &freeSeats );
	}

	pthread_mutex_unlock( &(ref->l_lotacao) );
}

void Onibus::chegaDestino( Onibus* ref ) {
	pthread_mutex_lock( &(ref->l_chegouDestino) );
	ref->chegouDestino = true;
	pthread_cond_broadcast( &(ref->podeDescer) );
	pthread_mutex_unlock( &(ref->l_chegouDestino) );
}