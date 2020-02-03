/*
 * wireless_base.h
 *
 *  Created on: 25.01.2020
 *      Author: Mati
 */

#ifndef WIRELESS_BASE_H_
#define WIRELESS_BASE_H_

#include "stm32f3xx_hal.h"

#define RPI_IRM 		0xAA
#define IRM_RPI 		0xBB

enum class communicationtype_t{
	point_to_point,
	wifi
};

struct wframeRx_s {
	uint8_t start;
	uint8_t target;
	uint8_t id;
	uint8_t cmd;
	uint8_t subcmd1;
	uint8_t subcmd2;
	uint8_t subcmd3;
	uint8_t subcmd4;
	uint8_t free[23];
	uint8_t crc8;
};

struct wframeTx_s {
	uint8_t start;
	uint8_t target;
	uint8_t id;
	float val; //<<Make sure this is 32bit
	char  desc[24];
	uint8_t crc8;
};

union txframe_u{
	uint8_t 	buffer[32];
	wframeTx_s	values;
};

union rxframe_u{
	uint8_t 	buffer[32];
	wframeRx_s 	values;
};

class Wireless{

protected:

	communicationtype_t 			commType;

	communicationtype_t& 			typeGet(void);


public:

	Wireless(){};

	virtual ~Wireless(){};

};




#endif /* WIRELESS_BASE_H_ */
