


#include <IrrigationController.h>



void Pump::start(void){
	//TODO: add timing restrictions
	if(this->idletimeGet() > this->idletimeRequiredSeconds){
		this->stateSet(state_t::running);
		this->idletimeReset();
	}
	else{
		this->stateSet(state_t::waiting);
	}
}

void Pump::stop(void){

	if(this->stateGet() == state_t::running || this->stateGet() == state_t::init){
		this->stateSet(state_t::stopped);
		this->runtimeReset();
	}
}

Pump::state_t Pump::stateGet(void){
	return this->state;
}

void Pump::stateSet(state_t state){
	this->state = state;
}

bool Pump::isRunning(void){
	return stateGet() == state_t::running ? true : false;
}

void Pump::step(double dt){

	if(this->isRunning() == true){
		this->runtimeIncrease(dt);
	}
	else{
		this->runtimeReset();
		this->idletimeIncrease(dt);
	}
}

void Pump::runtimeReset(void){
	this->runtimeSeconds = 0.0;
}

void Pump::runtimeIncrease(double dt){
	this->runtimeSeconds += dt;
}

double Pump::runtimeGet(void){
	return this->runtimeSeconds;
}

void Pump::idletimeReset(void){
	this->idletimeSeconds = 0.0;
}

void Pump::idletimeIncrease(double dt){
	this->idletimeSeconds += dt;
}

double Pump::idletimeGet(void){
	return this->idletimeSeconds;
}

string Pump::descriptionGet(void){
	return this->description;
}


bool Controller::run(void){


	string name = p1->descriptionGet();
	//p2->incrementRunningTime();
	//p3->incrementRunningTime();

	return true;
}


bool Tank::temperatureSet(double temperature){

	this->temperature = temperature;

	if(temperature < 0.0){
		this->_isOK = false;
		this->stateSet(state_t::frozen);
	}
	else{
		this->_isOK = true;
		this->stateSet(state_t::liquid);
	}

	return this->_isOK;
}

double Tank::temperatureGet(void){
	return this->temperature;
}

bool Tank::waterlevelSet(uint8_t waterlevel){

	this->waterlevel = waterlevel;
	//todo: constrain to 0-100

	if (this->waterlevel < 10){
		this->_isOK = false;
	}
	else this->_isOK = true;

	return this->_isOK;
}

uint8_t Tank::waterlevelGet(void){
	return this->waterlevel;
}

bool Tank::checkStateOK(void){
	return this->_isOK;
}

Tank::state_t Tank::stateGet(void){
	return this->state;
}
void Tank::stateSet(state_t state){
	this->state = state;
}
