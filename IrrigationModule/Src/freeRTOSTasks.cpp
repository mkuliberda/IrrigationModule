/*
 * freeRTOSTasks.cpp
 *
 *  Created on: 30.11.2019
 *      Author: Mati
 */

/*Pelargonie co drugi dzie�
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
      LEDToggle(10);
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

	const double tank1HeightMeters = 0.43;
	const double tank1VolumeLiters = 5.0;

	const struct gpio_s pump1gpio = {PUMP1_GPIO_Port, PUMP1_Pin};
	const struct gpio_s pump1led  = {PUMP1LD_GPIO_Port, PUMP1LD_Pin};
	const struct gpio_s pump2gpio = {PUMP2_GPIO_Port, PUMP2_Pin};
	const struct gpio_s pump2led  = {PUMP2LD_GPIO_Port, PUMP2LD_Pin};
	const struct gpio_s pump3gpio = {PUMP3_GPIO_Port, PUMP3_Pin};
	const struct gpio_s pump3led  = {PUMP3LD_GPIO_Port, PUMP3LD_Pin};

	const struct gpio_s opticalwaterlevelsensor1gpio = {T1_WATER_LVL_H_GPIO_Port, T1_WATER_LVL_H_Pin};
	const struct gpio_s opticalwaterlevelsensor2gpio = {T1_WATER_LVL_L_GPIO_Port, T1_WATER_LVL_L_Pin};
	const array<struct gpio_s, 2> opticalwaterlevelsensorGPIO = {opticalwaterlevelsensor1gpio, opticalwaterlevelsensor2gpio};
	const array<float,10> opticalwaterlevelsensorspos = {0.11, 0.87};

	BinaryPump *pump1 = new BinaryPump();
	pump1->init(3, 10, pump1gpio, pump1led);

	BinaryPump *pump2 = new BinaryPump();
	pump2->init(3, 10, pump2gpio, pump2led);

	BinaryPump *pump3 = new BinaryPump();
	pump3->init(3, 10, pump3gpio, pump3led);

	WaterTank *tank1 = new WaterTank(tank1HeightMeters, tank1VolumeLiters);

	tank1->init(opticalwaterlevelsensorspos);

	using index_t = array<int,2>::size_type;
	for (index_t i {0}; i<opticalwaterlevelsensorspos.size(); ++i)
	{
		tank1->vWLSensors[i].init(waterlevelsensorsubtype_t::fixed, opticalwaterlevelsensorGPIO[i]);
	}

	Plant *plant1 = new Plant("Pelargonia");
	Plant *plant2 = new Plant("Surfinia");
	Plant *plant3 = new Plant("Trawa");

	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.1 * TASK_FREQ_MULTIPLIER; //<1Hz
	xLastWakeTime=xTaskGetTickCount();

	double dt_seconds = xFrequencySeconds/1000.0f;

	bool test_cmd = true;
	bool cmd_consumed = false;
	uint32_t test_cnt = 0;



    for( ;; )
    {
    	if(test_cnt < 200)
    	{
    		test_cmd = true;
        	pump1->run(dt_seconds,test_cmd, cmd_consumed);
    		//pump1->forcestart();
    	}
    	else if(test_cnt >= 200 && test_cnt < 400)
    	{
    		if(test_cnt == 110) cmd_consumed = false;
    		test_cmd = false;
        	pump1->run(dt_seconds,test_cmd, cmd_consumed);
    		//pump1->forcestop();
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



