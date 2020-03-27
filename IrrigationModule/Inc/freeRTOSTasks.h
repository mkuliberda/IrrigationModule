/**
 * FreeRTOSTasks.h
 *
 *  Created on: 30.11.2019
 *      Author: Mati
 *
 *
 **/

#ifndef FREERTOSTASKS_H_
#define FREERTOSTASKS_H_

#include "cmsis_os.h"
#include "gpio.h"

#ifdef __cplusplus
 extern "C" {
#endif


#define TASK_FREQ_MULTIPLIER 1000

void vADCReadTask( void *pvParameters );
void vLEDFlashTask( void *pvParameters );
void vUserButtonCheckTask( void *pvParameters );
void vIrrigationControlTask( void *pvParameters );
void vStatusNotifyTask( void *pvParameters );
void vWirelessCommTask( void *pvParameters );


#ifdef __cplusplus
 }
#endif

#endif /* FREERTOSTASKS_H_ */
