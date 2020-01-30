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
	uint8_t start 	= RPI_IRM;
	uint8_t target 	= 0;
	uint8_t id 		= 0;
	uint8_t cmd  	= 0;
	uint8_t subcmd1 = 0;
	uint8_t subcmd2 = 0;
	uint8_t subcmd3 = 0;
	uint8_t subcmd4 = 0;
	uint8_t free[23];
	uint8_t crc8;
};

struct wframeTx_s {
	uint8_t start 	= IRM_RPI;
	uint8_t target 	= 0;
	uint8_t id 		= 0;
	float val 		= 0;
	char  desc[24];
	uint8_t crc8	= 0;
};

union wprotocol_u{
	uint8_t 	buffer[32];
	wframeRx_s 	rx;
	wframeTx_s	tx;

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
