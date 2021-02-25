/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "led.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
static int threshold = 10000;
int cont = 0;
uint8_t contador=0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for acelerometro */
osThreadId_t acelerometroHandle;
const osThreadAttr_t acelerometro_attributes = {
  .name = "acelerometro",
  .priority = (osPriority_t) osPriorityRealtime2,
  .stack_size = 128 * 4
};
/* Definitions for ledazul */
osThreadId_t ledazulHandle;
const osThreadAttr_t ledazul_attributes = {
  .name = "ledazul",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for interrup */
osThreadId_t interrupHandle;
const osThreadAttr_t interrup_attributes = {
  .name = "interrup",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 128 * 4
};
/* Definitions for Queue01 */
osMessageQueueId_t Queue01Handle;
const osMessageQueueAttr_t Queue01_attributes = {
  .name = "Queue01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void acelerometro_task(void *argument);
void ledazul_task(void *argument);
void interrup_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}
/* USER CODE END 2 */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
}

__weak void PostSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
}
/* USER CODE END PREPOSTSLEEP */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Queue01 */
  Queue01Handle = osMessageQueueNew (3, sizeof(uint8_t), &Queue01_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */


  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of acelerometro */
  acelerometroHandle = osThreadNew(acelerometro_task, NULL, &acelerometro_attributes);

  /* creation of ledazul */
  ledazulHandle = osThreadNew(ledazul_task, NULL, &ledazul_attributes);

  /* creation of interrup */
  interrupHandle = osThreadNew(interrup_task, NULL, &interrup_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_acelerometro_task */
/**
  * @brief  Function implementing the acelerometro thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_acelerometro_task */
void acelerometro_task(void *argument)
{
  /* USER CODE BEGIN acelerometro_task */
  /* Infinite loop */
  static uint8_t received2=0;
  int16_t Buffer[3]={0,0,0};
  	  for(;;)
	  {
		  osMessageQueueGet(Queue01Handle, &received2, 0, portMAX_DELAY);
		  if (received2 == 1)
		  {
			  /*tick_inicial=*/
			  BSP_ACCELERO_GetXYZ(Buffer);
			  if (fabs(Buffer[0]) > threshold){
			  LED_Verde_ON();
			  }
			  else{
			  LED_Verde_OFF();
			  }
			  if (fabs(Buffer[1]) > threshold){
			  LED_Naranja_ON();
			  }
			  else{
			  LED_Naranja_OFF();
			  }
			  if (fabs(Buffer[2]) > threshold){
			  LED_Rojo_ON();
			  }
			  else{
			  LED_Rojo_OFF();
			  }
			  cont+=1;
			  osDelay(1);
		  }
	  }

  /* USER CODE END acelerometro_task */
}

/* USER CODE BEGIN Header_ledazul_task */
/**
* @brief Function implementing the ledazul thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ledazul_task */
void ledazul_task(void *argument)
{
  /* USER CODE BEGIN ledazul_task */
  /* Infinite loop */
  static uint8_t received=0; /*en freertos, no hace falta poner static porque tiene memoria reservada*/

  for(;;)
  {
	  osMessageQueueGet(Queue01Handle, &received, 0, portMAX_DELAY);
	if (received == 1)
	{
		LED_Azul_BLINK();
	}
	  osDelay(10);
  }
  /* USER CODE END ledazul_task */
}

/* USER CODE BEGIN Header_interrup_task */
/**
* @brief Function implementing the interrup thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_interrup_task */
void interrup_task(void *argument)
{
  /* USER CODE BEGIN interrup_task */
  /* Infinite loop */
	int pulsador = 0;
  for(;;)
  {
	  if(pulsador == 1)
			  {
		  	  	  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 0){
		  	  		pulsador=0;
		  	  		contador=contador+1;

			  	  	  if (contador == 2)
			  	  	  {
			  	  		  contador=0;
			  			  LED_Azul_OFF();
						  LED_Verde_OFF();
						  LED_Naranja_OFF();
						  LED_Rojo_OFF();
			  	  	  }
		  	  	  }
			  }
	  else {
		  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1){
		  		  	pulsador=1;
		  }
	  }


	  osMessageQueuePut(Queue01Handle, &contador, 0, portMAX_DELAY);
	  osMessageQueuePut(Queue01Handle, &contador, 0, portMAX_DELAY);
	  uint32_t tick_final = osKernelGetTickCount();
	  osDelay(5);
  }
  /* USER CODE END interrup_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
