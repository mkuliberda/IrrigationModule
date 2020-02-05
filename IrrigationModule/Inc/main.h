/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
 extern "C" {
#endif

  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define TANK1STATUS_BUFFER_LENGTH 1
#define PUMPSSTATUS_BUFFER_LENGTH 1
#define PLANTSHEALTH_BUFFER_LENGTH 4
#define BATTERY_BUFFER_LENGTH 1
#define ADCVALUES_BUFFER_LENGTH 9
#define EXTCMDS_BUFFER_LENGTH 10
#define SYSSTATUS_BUFFER_LENGTH 1

#define DRDY_Pin GPIO_PIN_2
#define DRDY_GPIO_Port GPIOE
#define CS_I2C_SPI_Pin GPIO_PIN_3
#define CS_I2C_SPI_GPIO_Port GPIOE
#define MEMS_INT3_Pin GPIO_PIN_4
#define MEMS_INT3_GPIO_Port GPIOE
#define MEMS_INT4_Pin GPIO_PIN_5
#define MEMS_INT4_GPIO_Port GPIOE
#define OSC32_IN_Pin GPIO_PIN_14
#define OSC32_IN_GPIO_Port GPIOC
#define OSC32_OUT_Pin GPIO_PIN_15
#define OSC32_OUT_GPIO_Port GPIOC
#define NRF24_SCK_Pin GPIO_PIN_9
#define NRF24_SCK_GPIO_Port GPIOF
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOF
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOF
#define SOIL_SENSOR4_Pin GPIO_PIN_0
#define SOIL_SENSOR4_GPIO_Port GPIOC
#define SOIL_SENSOR5_Pin GPIO_PIN_1
#define SOIL_SENSOR5_GPIO_Port GPIOC
#define SOIL_SENSOR6_Pin GPIO_PIN_2
#define SOIL_SENSOR6_GPIO_Port GPIOC
#define SOIL_SENSOR7_Pin GPIO_PIN_3
#define SOIL_SENSOR7_GPIO_Port GPIOC
#define SOIL_SENSOR8_Pin GPIO_PIN_2
#define SOIL_SENSOR8_GPIO_Port GPIOF
#define B1_Pin GPIO_PIN_0
#define B1_GPIO_Port GPIOA
#define BAT_VOLTAGE_Pin GPIO_PIN_1
#define BAT_VOLTAGE_GPIO_Port GPIOA
#define SOIL_SENSOR1_Pin GPIO_PIN_2
#define SOIL_SENSOR1_GPIO_Port GPIOA
#define SOIL_SENSOR2_Pin GPIO_PIN_3
#define SOIL_SENSOR2_GPIO_Port GPIOA
#define SOIL_SENSOR3_Pin GPIO_PIN_4
#define SOIL_SENSOR3_GPIO_Port GPIOF
#define SPI1_SCK_Pin GPIO_PIN_5
#define SPI1_SCK_GPIO_Port GPIOA
#define SPI1_MISO_Pin GPIO_PIN_6
#define SPI1_MISO_GPIO_Port GPIOA
#define SPI1_MISOA7_Pin GPIO_PIN_7
#define SPI1_MISOA7_GPIO_Port GPIOA
#define PUMP2LD_Pin GPIO_PIN_8
#define PUMP2LD_GPIO_Port GPIOE
#define PUMP1LD_Pin GPIO_PIN_9
#define PUMP1LD_GPIO_Port GPIOE
#define PUMP3LD_Pin GPIO_PIN_10
#define PUMP3LD_GPIO_Port GPIOE
#define LD7_Pin GPIO_PIN_11
#define LD7_GPIO_Port GPIOE
#define LD9_Pin GPIO_PIN_12
#define LD9_GPIO_Port GPIOE
#define LD10_Pin GPIO_PIN_13
#define LD10_GPIO_Port GPIOE
#define LD8_Pin GPIO_PIN_14
#define LD8_GPIO_Port GPIOE
#define LD6_Pin GPIO_PIN_15
#define LD6_GPIO_Port GPIOE
#define NRF24_IRQ_Pin GPIO_PIN_10
#define NRF24_IRQ_GPIO_Port GPIOB
#define NRF24_NSS_Pin GPIO_PIN_12
#define NRF24_NSS_GPIO_Port GPIOB
#define NRF24_CE_Pin GPIO_PIN_13
#define NRF24_CE_GPIO_Port GPIOB
#define NRF24_MISO_Pin GPIO_PIN_14
#define NRF24_MISO_GPIO_Port GPIOB
#define NRF24_MOSI_Pin GPIO_PIN_15
#define NRF24_MOSI_GPIO_Port GPIOB
#define PUMP1_Pin GPIO_PIN_10
#define PUMP1_GPIO_Port GPIOD
#define PUMP2_Pin GPIO_PIN_11
#define PUMP2_GPIO_Port GPIOD
#define PUMP3_Pin GPIO_PIN_12
#define PUMP3_GPIO_Port GPIOD
#define PUMP4_Pin GPIO_PIN_13
#define PUMP4_GPIO_Port GPIOD
#define T1_WATER_LVL_H_Pin GPIO_PIN_6
#define T1_WATER_LVL_H_GPIO_Port GPIOC
#define T1_WATER_LVL_L_Pin GPIO_PIN_7
#define T1_WATER_LVL_L_GPIO_Port GPIOC
#define DS18B20_1_Pin GPIO_PIN_8
#define DS18B20_1_GPIO_Port GPIOC
#define DM_Pin GPIO_PIN_11
#define DM_GPIO_Port GPIOA
#define DP_Pin GPIO_PIN_12
#define DP_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_7
#define I2C1_SDA_GPIO_Port GPIOB
#define MEMS_INT1_Pin GPIO_PIN_0
#define MEMS_INT1_GPIO_Port GPIOE
#define MEMS_INT2_Pin GPIO_PIN_1
#define MEMS_INT2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

//void _Error_Handler(char *, int);

//#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 
#ifdef __cplusplus
 }
#endif
#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
