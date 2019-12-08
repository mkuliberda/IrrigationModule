/*
 * freeRTOSTasks.cpp
 *
 *  Created on: 30.11.2019
 *      Author: Mati
 */

/*Pelargonie co drugi dzieñ
 * Surfinie codziennie
 * Trawa raz na 3 dni
 */

#include <freeRTOSTasks.h>
#include <irrigation.h>
#include <plants.h>
#include "gpio.h"
#include "main.h"

SemaphoreHandle_t xUserButtonSemaphore = NULL;


void vLEDFlashTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 1 * TASK_FREQ_MULTIPLIER; //<1Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
      LEDToggle(5);
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}

void vUserButtonCheckTask(void *pvParameters )
{
	vSemaphoreCreateBinary(xUserButtonSemaphore);

	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<10Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}


void vIrrigationControlTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
	xLastWakeTime=xTaskGetTickCount();

	GenericPump *pump1 = new GenericPump("Surfinie", 2, 10, PUMP1_GPIO_Port, PUMP1_Pin);
	GenericPump *pump2 = new GenericPump("Trawa", 2, 10, PUMP2_GPIO_Port, PUMP2_Pin);
	GenericPump *pump3 = new GenericPump("Pelargonie", 2, 15, PUMP3_GPIO_Port, PUMP3_Pin);

	WaterTank *tank1 = new WaterTank(SENSORSAMOUNT_TANK1);

	Plant *plant1 = new Plant("Pelargonia");
	Plant *plant2 = new Plant("Surfinia");
	Plant *plant3 = new Plant("Trawa");



    for( ;; )
    {
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

    delete pump1; delete pump2; delete pump3;
    delete tank1;
    delete plant1; delete plant2; delete plant3;

}
void vStatusNotifyTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 1 * TASK_FREQ_MULTIPLIER; //<1Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

}
void vWirelessCommTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.1 * TASK_FREQ_MULTIPLIER; //<10Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

}



