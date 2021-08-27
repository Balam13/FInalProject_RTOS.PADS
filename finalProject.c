/*
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    finalProject.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include <math.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "fsl_port.h"

/* TODO: insert other include files here. */
#include "nokiaLCD.h"
#include "Freertos.h"
#include "task.h"
#include "semphr.h"
#include "freertos_spi.h"
#include "event_groups.h"
#include "queue.h"



#define ALARM_SECOND (1UL << 0UL)
#define ALARM_MINUTE (1UL << 1UL)
#define ALARM_HOUR   (1UL << 2UL)


uint8_t alarmSecond = '0';
uint8_t alarmSeconds = '3';
uint8_t alarmMinute = '0';
uint8_t alarmMinutes = '0';
uint8_t alarmHour = '0';
uint8_t alarmHours = '0';


typedef struct
{
	SemaphoreHandle_t minutes_semaphore;
	SemaphoreHandle_t hours_semaphore;
}freertos_spi_handle_t;

typedef struct
{
	uint8_t unid;
	uint8_t dece;
}bi_val_t;

typedef enum{seconds_type, minutes_type, hours_type} time_types_t;
typedef struct
{
	time_types_t time_types;
	bi_val_t value;
}time_msg_t;

static freertos_spi_handle_t freertos_spi_handle[2] = {0};

freertos_spi_config_t masterConfig;

EventGroupHandle_t xEventGroup;


void taskLCDInit(void *args);

void taskSeconds(void * args);

void taskMinutes(void * args);

void taskHours(void * args);

void taskAlarm( void *pvParameters );

void taskPrint (void *pvParameters);


/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    static QueueHandle_t mailbox;

    mailbox =xQueueCreate(8,sizeof(time_msg_t*));

    xEventGroup = xEventGroupCreate();

    xTaskCreate(taskLCDInit, "Init LCD", 100, NULL, 4, NULL);

    xTaskCreate(taskPrint, "Print", 300, ((void*) &mailbox), 3, NULL);

    xTaskCreate(taskSeconds, "Send Sec", 200, ((void*) &mailbox), 2, NULL);

    xTaskCreate(taskMinutes, "Send Min", 200, ((void*) &mailbox), 2, NULL);

    xTaskCreate(taskHours, "Send Hr", 200,((void*) &mailbox), 2, NULL);

    xTaskCreate(taskAlarm, "Exe Alarm", 300, NULL, 1, NULL);


    vTaskStartScheduler();


    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
//    	nokiaLCD_backlight(1);
//    	delay(500);
//    	nokiaLCD_backlight(0);
//    	delay(500);
    }
    return 0 ;
}



void taskLCDInit(void *args)
{
	for(;;)
	{
		nokiaLCD_initialise();

		nokiaLCD_clearDisplay(0);

		vTaskDelay(portMAX_DELAY);
	}
}


void taskSeconds(void * args)
{
	uint8_t seconds = '0';
	uint8_t second = '0';

	time_msg_t cop_second;
	time_msg_t *pcop_second;
	QueueHandle_t mailbox = *((QueueHandle_t*)args);

//	vTaskDelay(pdMS_TO_TICKS(500));

	freertos_spi_handle[masterConfig.spi_number].minutes_semaphore = xSemaphoreCreateBinary();
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;


	for(;;)
	{

		if(second > '5')
		{
			second = '0';
		}

		if (seconds > '9')
		{
			seconds = '0';
			second++;
		}

		if (second == '6' && seconds == '0')
		{
			seconds = '0';

			second = '0';

			xSemaphoreGiveFromISR(freertos_spi_handle[freertos_spi0].minutes_semaphore, &xHigherPriorityTaskWoken);
		}

		if (((second == alarmSecond) & (seconds == alarmSeconds)) != 0)
		{

			xEventGroupSetBits(xEventGroup, ALARM_SECOND);

			PRINTF("Si entro");
		}


		//Llenado de copia
		cop_second.time_types = seconds_type;
		cop_second.value.unid = second;
		cop_second.value.dece = seconds;
		pcop_second = pvPortMalloc(sizeof(cop_second));
		*pcop_second = cop_second;
		//Envio de la cola
		xQueueSend(mailbox,&pcop_second,0);

		seconds++;

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}


void taskMinutes(void * args)
{
	uint8_t minutes = '0';
	uint8_t minute = '0';
	time_msg_t cop_minute;
	time_msg_t *pcop_minute;
	QueueHandle_t mailbox = *((QueueHandle_t*)args);
//	vTaskDelay(pdMS_TO_TICKS(500));

	freertos_spi_handle[masterConfig.spi_number].hours_semaphore = xSemaphoreCreateBinary();
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	for(;;)
	{

		if(minute > '5')
		{
			minute = '0';
		}

		if (minutes > '9')
		{
			minutes = '0';

			minute++;
		}

		if (minute == '6' && minutes == '0')
		{
			minutes = '0';

			minute = '0';

			xSemaphoreGiveFromISR(freertos_spi_handle[freertos_spi0].hours_semaphore, &xHigherPriorityTaskWoken);
		}


		if ((minute == alarmMinute) & (minutes == alarmMinutes))
		{

			xEventGroupSetBits(xEventGroup, ALARM_MINUTE );
			PRINTF("Si entro min");
		}


		//Llenado de copia
		cop_minute.time_types = minutes_type;
		cop_minute.value.unid = minute;
		cop_minute.value.dece = minutes;

		pcop_minute = pvPortMalloc(sizeof(cop_minute));
		*pcop_minute = cop_minute;
		//Envio de la cola
		xQueueSend(mailbox,&pcop_minute,0);

		minutes++;

		xSemaphoreTake(freertos_spi_handle[masterConfig.spi_number].minutes_semaphore, portMAX_DELAY);

		}
}

void taskHours(void * args)
{
	uint8_t hours = '0';
	uint8_t hour = '0';

	time_msg_t cop_hour;
	time_msg_t *pcop_hour;
	QueueHandle_t mailbox = *((QueueHandle_t*)args);

//	vTaskDelay(pdMS_TO_TICKS(500));

	for(;;)
	{

		if(hour > '5')
		{
			hour = '0';
		}

		if (hours > '9')
		{
			hours = '0';

			hour++;
		}

		if (hour == '2' && hours == '4')
		{
			hours = '0';

			hour = '0';
		}

		if ((hour == alarmHour) & (hours == alarmHours))
		{

			xEventGroupSetBits(xEventGroup, ALARM_HOUR );
			PRINTF("Si entro ho");
		}


		//Llenado de copia
		cop_hour.time_types = hours_type;
		cop_hour.value.unid = hour;
		cop_hour.value.dece = hours;

		pcop_hour = pvPortMalloc(sizeof(cop_hour));
		*pcop_hour = cop_hour;
		//Envio de la cola
		xQueueSend(mailbox,&pcop_hour,0);

		hours++;

		xSemaphoreTake(freertos_spi_handle[masterConfig.spi_number].hours_semaphore, portMAX_DELAY);

	}
}


void taskAlarm( void *pvParameters )
{
	PRINTF("Si entro tambien");

	const EventBits_t xBitsToWaitFor = (ALARM_SECOND | ALARM_MINUTE | ALARM_HOUR); // );
	EventBits_t xAlarmValue;

		for( ;; )
		{
			/* Block to wait for event bits to become set within the event group. */
			xAlarmValue = xEventGroupWaitBits( /* The event group to read. */
			              xEventGroup,

                        /* Bits to test. */
                        xBitsToWaitFor,

                        /* Clear bits on exit if the
                        unblock condition is met. */
                        pdTRUE,

                        /* Do wait for all bits. */
                        pdTRUE,

                        /* Don't time out. */
                        portMAX_DELAY);

    /* Print a message for each bit that was set. */
			if((xAlarmValue & (ALARM_SECOND | ALARM_MINUTE | ALARM_HOUR)) != 0 )
			{
				char alarm1 = 'A';
				char alarm2 = 'L';
				char alarm3 = 'A';
				char alarm4 = 'R';
				char alarm5 = 'M';
				char alarm6 = 'A';

				nokiaLCD_setChar(alarm1, 10, 15, 6);
				nokiaLCD_setChar(alarm2, 16, 15, 7);
				nokiaLCD_setChar(alarm3, 22, 15, 7);
				nokiaLCD_setChar(alarm4, 28, 15, 7);
				nokiaLCD_setChar(alarm5, 34, 15, 7);
				nokiaLCD_setChar(alarm6, 40, 15, 7);

				nokiaLCD_updateDisplay();

//				vTaskDelay(pdMS_TO_TICKS(1000));
//
//				nokiaLCD_clearDisplay(0);

			}

			}
}


void taskPrint (void *pvParameters)
{

		nokiaLCD_setChar('0', 0, 0, 1);
		nokiaLCD_setChar('0', 8, 0, 1);
		nokiaLCD_setChar('0', 20, 0, 1);
		nokiaLCD_setChar('0', 28, 0, 1);
		nokiaLCD_setChar('0', 40, 0, 1);
		nokiaLCD_setChar('0', 48, 0, 1);

		nokiaLCD_updateDisplay();

	QueueHandle_t mailbox = *((QueueHandle_t*)pvParameters);
	time_msg_t *pcop;


	PRINTF("Aqui estoy");

	for(;;)
	{
       xQueueReceive(mailbox, &pcop, portMAX_DELAY);

       switch (pcop->time_types)
       {
            case seconds_type:
            	nokiaLCD_setChar(pcop->value.unid, 40, 0, 1);
            	nokiaLCD_setChar(pcop->value.dece, 48, 0, 1);
            	nokiaLCD_updateDisplay();
            	break;

            case minutes_type:
            	nokiaLCD_setChar(pcop->value.unid, 20, 0, 1);
            	nokiaLCD_setChar(pcop->value.dece, 28, 0, 1);
            	nokiaLCD_updateDisplay();
            	break;

            case hours_type:
            	nokiaLCD_setChar(pcop->value.unid, 0, 0, 1);
            	nokiaLCD_setChar(pcop->value.dece, 8, 0, 1);
            	nokiaLCD_updateDisplay();
            	break;

            default:
            	nokiaLCD_setChar('0', 0, 0, 1);
            	nokiaLCD_setChar('0', 8, 0, 1);
            	nokiaLCD_setChar('0', 20, 0, 1);
            	nokiaLCD_setChar('0', 28, 0, 1);
            	nokiaLCD_setChar('0', 40, 0, 1);
            	nokiaLCD_setChar('0', 48, 0, 1);
            	nokiaLCD_updateDisplay();

       }

       vPortFree(pcop);
	}
}





