#include <PumpController.h>




bool Pump::isRunning(void){
	return this->_running;
}
void Pump::turnON(void){
		this->_running = true;
}
void Pump::turnOFF(void){
		this->_running = false;
}


