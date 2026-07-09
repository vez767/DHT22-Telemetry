/*
 * hcrs04.c
 *
 *  Created on: 7 Jul 2026
 *      Author: vez767
 */

#include <stdint.h>
#include "hcsr04.h"

volatile uint32_t capture_start = 0;
volatile uint32_t capture_duration = 0;
volatile uint32_t capture_end = 0;

void HCSR04_Init(void){

	RCC_AHB1ENR |= (1U << 0); // GPIOA-EN

	//PINS: PA9 - OUTPUT & PA1 - ALTERNATE FUNCTION
	GPIOA_MODER &= ~((3U << 2) | (3U << 18));
	GPIOA_MODER |= ((2U << 2) | (1U << 18));

	GPIOA_AFRL &= ~(0xFU << 4); // AFRL1[3:0]
	GPIOA_AFRL |= (1U << 4);

					/*TIM2 Config*/
	RCC_APB1ENR |= (1U << 0);

	TIM2_PSC = 15U;
	TIM2_ARR = 0xFFFFFFFF;
	TIM2_EGR |= (1 << 0);

	// ECHO Logic
	TIM2_CCMR1 &= ~(3U << 8);
	TIM2_CCMR1 |= (1U << 8); //  Capture/Compare 2 selection: Input - TI2

	TIM2_CCER |= ((1U << 5) | (1U << 7) | (1U << 4)); // CC2P/CC2NP: [11] - Sensitive to Both Edges AND CC2E - EN

	TIM2_DIER |= (1U << 2); // CC2IE: Capture/Compare 2 interrupt enable

	TIM2_CR1 |= (1 << 0);

	NVIC_IPR7 &= ~(0xFFU << 0);
	NVIC_IPR7 |= ((5U << 4) << 0);

	NVIC_ISER0 |= (1U << 28);
}

void TIM2_IRQHandler(void){

	if(TIM2_SR & (1U << 2)){

		if(GPIOA_IDR & (1U << 1)){
			// Rising Edge
			capture_start = TIM2_CCR2;

		}else{
				// Falling Edge
			capture_end = TIM2_CCR2;
			capture_duration = capture_end - capture_start;
		}

		TIM2_SR &= ~(1U << 2); // Clear Flag
	}


}
