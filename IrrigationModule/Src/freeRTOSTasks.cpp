/*
 * freeRTOSTasks.cpp
 *
 *  Created on: 30.11.2019
 *      Author: Mati
 */

#include <freeRTOSTasks.h>
#include "gpio.h"

SemaphoreHandle_t xUserButtonSemaphore = NULL;
SemaphoreHandle_t xTankValidSemaphore = NULL;
SemaphoreHandle_t xPump1RunSemaphore = NULL;
SemaphoreHandle_t xPump2RunSemaphore = NULL;
SemaphoreHandle_t xPump3RunSemaphore = NULL;
SemaphoreHandle_t xTankOKSemaphore = NULL;

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

void vPlantsCheckTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 360 * TASK_FREQ_MULTIPLIER; //<1h
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

}

void vTankCheckTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 10 * TASK_FREQ_MULTIPLIER; //<0.1Hz
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

    for( ;; )
    {
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

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
void vCommandsGetTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.1 * TASK_FREQ_MULTIPLIER; //<10Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

}


