/*
 * usart.c
 *
 *  Created on: 22 Jul 2026
 *      Author: vez767
 */

#include "stm32f4xx.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

#define RX_BUFFER_SIZE	512

volatile uint8_t gps_rx_buffer[RX_BUFFER_SIZE];
extern TaskHandle_t xGpsTaskHandle;

void USART6_IRQHandler(void){

	if(USART6->SR & USART_SR_IDLE){

		volatile uint32_t compulsory_read = USART6->DR;
		(void)compulsory_read;

		BaseType_t xHigherPriorityTask = pdFALSE;
		vTaskNotifyGiveFromISR(xGpsTaskHandle, &xHigherPriorityTask);
		portYIELD_FROM_ISR(xHigherPriorityTask);

	}
}

void USART2_Init(void){

RCC->APB1ENR |= (1U << 17); // USART 2
RCC->AHB1ENR |= (1U << 0);	// GPIOAEN

GPIOA->MODER &= ~(0xFU << 4);//  CLEAR PA2 & 3
GPIOA->MODER |= (0xAU << 4); // PA2 & PA3 - AF (0b1010)

GPIOA->AFR[0] &= ~(0xFFU << 8);
GPIOA->AFR[0] |= (0x77U << 8);

GPIOA->OSPEEDR &= ~(3U << 4);
GPIOA->OSPEEDR |= (2U << 4); // HIGH - 0b10

USART2->BRR = (8U << 4) | (11U << 0);

USART2->CR1 |= (1U << 13) | (1U << 3) | (1U << 2); // USARTEN [13] : TE [3] : RE [2]
}

void USART2_Write(int ch){

	while(!(USART2->SR & (1U << 7))){

	}

	USART2->DR = ch;
}

/*
 * Requirement: Configure USART6 to receive variable-length NMEA GPS sentences.
 * Constraint: Offload CPU overhead by linking Rx to DMA2 Stream 1 in circular mode.
 */
void GPS_USART6_Init(void){

	RCC->AHB1ENR |= (1U << 0) | (1U << 22); // GPIOAEN | DMA2EN
	RCC->APB2ENR |= (1U << 5); //USART6EN

	GPIOA->MODER &= ~(3U << 24);//  CLEAR PA12
	GPIOA->MODER |= (2U << 24); // PA12 - AF (0b10)

	GPIOA->AFR[1] &= ~(0xFU << 16); //Rx
	GPIOA->AFR[1] |= (0x8U << 16);

	DMA2_Stream1->CR |= (1U << 8) | (1U << 10) | (5U << 25); // MINC | Circular mode | Channel; 5

	DMA_Transfer((uint32_t) &(USART6->DR),(uint32_t) &(gps_rx_buffer[0]), RX_BUFFER_SIZE);

	USART6->BRR = (104U << 4) | (3U << 0);

	USART6->CR1 |= (1U << 13) | (1U << 4) | (1U << 2); // USARTEN [13] | IDLEIE[4] | RE [2]
	USART6->CR3 |= (1U << 6);

	NVIC_SetPriority(USART6_IRQn, 5);
	NVIC_EnableIRQ(USART6_IRQn);

}

void DMA_Transfer(uint32_t source_dr, uint32_t dest_array, uint32_t buffer_size){

	DMA2_Stream1->M0AR = dest_array;
	DMA2_Stream1->PAR = source_dr;
	DMA2_Stream1->NDTR = buffer_size;

	DMA2_Stream1->CR |= (1U << 0);
}

void vUSART2_Task(void *pvParameters){
	static uint16_t tail = 0;
	static char parse_buffer[100];
	static uint8_t parse_index = 0;

	 while(1){
		  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		  uint16_t head = (RX_BUFFER_SIZE - DMA2_Stream1->NDTR) % RX_BUFFER_SIZE;

		  while (tail != head){
			  char c = gps_rx_buffer[tail];
		      tail = (tail + 1) % RX_BUFFER_SIZE;

		      if (c == '$') parse_index = 0;

		      if (parse_index < 99) {
		         parse_buffer[parse_index++] = c;
		       }

		      if (c == '\n') {
		    	  parse_buffer[parse_index] = '\0';

		    	  for(uint8_t i = 0; parse_buffer[i] != '\0'; i++) {
		    		  USART2_Write(parse_buffer[i]);
		    	  }
		      }
		  }
	 }
}


void USART2_Task_Init(void){
	xTaskCreate(vUSART2_Task, "vUSART2_Task", 256, NULL, 1, &xGpsTaskHandle);
}



