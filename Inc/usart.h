/*
 * usart.h
 *
 *  Created on: 22 Jul 2026
 *      Author: Windows
 */

#ifndef USART_H_
#define USART_H_

#include "stm32f4xx.h"

void USART2_Init(void);
void USART2_Write(int ch);
void GPS_USART6_Init(void);
void DMA_Transfer(uint32_t source_dr, uint32_t dest_array, uint32_t buffer_size);
void USART2_Task_Init(void);



#endif /* USART_H_ */
