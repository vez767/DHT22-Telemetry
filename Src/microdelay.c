/*
 * microdelay.c
 *
 *  Created on: 10 Jul 2026
 *      Author:vez767
 */

#include <stdint.h>
#include "microdelay.h"


void TIM3_Init(void){

	 RCC_APB1ENR |= (1U << 1); // TIM3 Enable

	 TIM3_ARR = 0xFFFF; // ARR Limit
	 TIM3_PSC = 15U; // Prescaler
	 TIM3_EGR |= (1U <<0);
	 TIM3_CR1 |= (1U << 0); // Enable Counter

 }


void delay_us(uint16_t us){

	 TIM3_CNT = 0;

	 while(TIM3_CNT < us);

 }
