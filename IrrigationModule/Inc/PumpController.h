/*
 * PumpController.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef PUMPCONTROLLER_H_
#define PUMPCONTROLLER_H_

class Pump{
	Pump(){_running = false;};
	~Pump();

public:
	bool isRunning(void);
	void turnON(void);
	void turnOFF(void);

private:
	bool _running;

};

class Controller{

};



#endif /* PUMPCONTROLLER_H_ */
