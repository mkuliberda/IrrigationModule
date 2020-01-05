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
#include <utilities.h>
#include <plants.h>
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include <cstring>
#include "main.h"

#define TANK1STATUS_BUFFER_LENGTH 1
#define PUMPSSTATUS_BUFFER_LENGTH 1

SemaphoreHandle_t xUserButtonSemaphore = NULL;
xQueueHandle tank1StatusQueue;
xQueueHandle pumpsStatusQueue;



void vLEDFlashTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 1 * TASK_FREQ_MULTIPLIER; //<1Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
      LEDToggle(10);
      vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}

void vUserButtonCheckTask(void *pvParameters )
{
	vSemaphoreCreateBinary(xUserButtonSemaphore);

	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.2 * TASK_FREQ_MULTIPLIER; //<5Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
    	if (UserButtonRead() == 1)
    	{
    		LEDToggle(8);
    		xSemaphoreGive(xUserButtonSemaphore);
    	}
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}


void vIrrigationControlTask( void *pvParameters )
{

	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.1 * TASK_FREQ_MULTIPLIER; //<10Hz
	xLastWakeTime=xTaskGetTickCount();

	double dt_seconds = xFrequencySeconds/1000.0f;

	tank1StatusQueue = xQueueCreate(TANK1STATUS_BUFFER_LENGTH, sizeof( uint32_t ) );
	pumpsStatusQueue = xQueueCreate(PUMPSSTATUS_BUFFER_LENGTH, sizeof( uint32_t ) );

	const double tank1HeightMeters = 0.43;
	const double tank1VolumeLiters = 5.0;
	const float WLSensorHighPositionMeters = 0.12;
	const float WLSensorLowPositionMeters = 0.38;
	uint32_t tank1Status = 0;
	uint32_t pumpsStatus = 0; //8 bits per pump

	const struct gpio_s pump1gpio = {PUMP1_GPIO_Port, PUMP1_Pin};
	const struct gpio_s pump1led  = {PUMP1LD_GPIO_Port, PUMP1LD_Pin};
	const struct gpio_s pump2gpio = {PUMP2_GPIO_Port, PUMP2_Pin};
	const struct gpio_s pump2led  = {PUMP2LD_GPIO_Port, PUMP2LD_Pin};
	const struct gpio_s pump3gpio = {PUMP3_GPIO_Port, PUMP3_Pin};
	const struct gpio_s pump3led  = {PUMP3LD_GPIO_Port, PUMP3LD_Pin};
	const struct gpio_s ds18b20gpio = {ONEWIRE_GPIO_Port, ONEWIRE_Pin};

	const struct gpio_s opticalwaterlevelsensor1gpio = {T1_WATER_LVL_H_GPIO_Port, T1_WATER_LVL_H_Pin};
	const struct gpio_s opticalwaterlevelsensor2gpio = {T1_WATER_LVL_L_GPIO_Port, T1_WATER_LVL_L_Pin};

	BinaryPump *pump1 = new BinaryPump();
	pump1->init(0, 2, 5, pump1gpio, pump1led);

	BinaryPump *pump2 = new BinaryPump();
	pump2->init(1, 3, 10, pump2gpio, pump2led);

	BinaryPump *pump3 = new BinaryPump();
	pump3->init(2, 3, 10, pump3gpio, pump3led);

	WaterTank *tank1 = new WaterTank(tank1HeightMeters, tank1VolumeLiters);
	tank1->init();
	tank1->waterlevelSensorAdd(waterlevelsensortype_t::WLS_optical);
	tank1->waterlevelSensorAdd(waterlevelsensortype_t::WLS_optical);
	tank1->temperatureSensorAdd(temperaturesensortype_t::ds18b20);

	tank1->vOpticalWLSensors[0].init(WLSensorHighPositionMeters, opticalwaterlevelsensor1gpio);
	tank1->vOpticalWLSensors[1].init(WLSensorLowPositionMeters, opticalwaterlevelsensor2gpio);
	tank1->vTemperatureSensors[0].init(ds18b20gpio, &htim7);

	//Plant *plant1 = new Plant("Pelargonia");
	//Plant *plant2 = new Plant("Surfinia");
	//Plant *plant3 = new Plant("Trawa");

	bool test_cmd = true;
	bool cmd_consumed = false;
	uint32_t test_cnt = 0;



    for( ;; )
    {
    	tank1->checkStateOK(tank1Status);
    	tank1Status+=test_cnt;
    	xQueueOverwrite( tank1StatusQueue, &tank1Status);

    	pumpStateEncode(pump1->status, pumpsStatus);
    	pumpStateEncode(pump2->status, pumpsStatus);
    	pumpStateEncode(pump3->status, pumpsStatus);
    	xQueueOverwrite( pumpsStatusQueue, &pumpsStatus);

    	if(test_cnt < 200)
    	{
    		test_cmd = true;
        	pump1->run(dt_seconds,test_cmd, cmd_consumed);
    	}
    	else if(test_cnt >= 200 && test_cnt < 400)
    	{
    		if(test_cnt == 250) cmd_consumed = false;
    		test_cmd = false;
        	pump1->run(dt_seconds,test_cmd, cmd_consumed);
    	}
    	else if(test_cnt >= 400)
		{
    		cmd_consumed = false;
    		test_cnt = 0;
		}
    	test_cnt++;
    	LEDToggle(6);
		vTaskDelayUntil(&xLastWakeTime, xFrequencySeconds);
    }

    delete pump1; delete pump2; delete pump3;
    delete tank1;
    //delete plant1; delete plant2; delete plant3;

}
void vStatusNotifyTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
	xLastWakeTime=xTaskGetTickCount();
	uint32_t tank1Status = 0;
	uint32_t pumpsStatus = 0;
	uint8_t message[] = "test\n";
	array<struct pumpstatus_s,4> a_pumpStatus;


    for( ;; )
    {
    	if(xUserButtonSemaphore != NULL)
    	{
		   if (xSemaphoreTake(xUserButtonSemaphore, (portTickType)2) == pdTRUE)
		   {
			   if(xQueueReceive( tank1StatusQueue, &tank1Status, 1 ) == pdPASS || xQueueReceive( pumpsStatusQueue, &pumpsStatus, 1 ) == pdPASS)
			   {
				   pumpStateDecode(a_pumpStatus, pumpsStatus);
				   sprintf((char*)message,"tank1: %d, pumps: %d\n",(unsigned int)tank1Status,(unsigned int)pumpsStatus);
				   HAL_UART_Transmit_DMA(&huart4,message,strlen((char*)message));
				   HAL_UART_DMAResume(&huart4);
				   LEDToggle(9);
			   }
		   }
		}

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



