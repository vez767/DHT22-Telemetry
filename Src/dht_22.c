#include <stdint.h>
#include "dht_22.h"


/**
 * @brief  Initializes Basic Timer 3 (TIM3) as a 1MHz microsecond stopwatch.
 * Requirement: Configure silicon to measure absolute microsecond durations for 1-Wire communication.
 * Constraint: Maximize CPU availability by preventing software-blocking delay loops; ensure the auto-reload register (ARR) ceiling is maximized to prevent early overflow.
 */


void DHT22_Start(void) {

	RCC_AHB1ENR |= (1U << 0); // GPIOA Enable

	 GPIOA_MODER &= ~(3U << 0);
	 GPIOA_MODER |= (1 << 0); // Output mode

	 GPIOA_ODR &= ~(1U << 0); // drop voltage in PA0

	 // Wait 1ms
	 Delay_us(1000);

	 GPIOA_MODER &= ~(3U << 0); // Input mode
}

 void Delay_us(uint16_t us){

	 TIM3_CNT = 0;

	 while(TIM3_CNT < us){

	 }

 }

void DHT22_Timer_Init(void){

	 RCC_APB1ENR |= (1U << 1); // TIM3 Enable

	 TIM3_ARR = 0xFFFF; // ARR Limit
	 TIM3_PSC = 15U; // Prescaler
	 TIM3_CR1 |= (1U << 0); // Enable Counter

 }

uint8_t DHT22_Check_Response(void){

	uint8_t response = 0;
	Delay_us(40);

	if(!(GPIOA_IDR & (1U << 0))){
		Delay_us(80);


	if(GPIOA_IDR & ((1U << 0))){
		response = 1;
		}
	}

	while (GPIOA_IDR & (1U << 0));

	return response;

}


