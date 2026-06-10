/*
 * crc.c
 *
 *  Created on: 2 May 2026
 *      Author: vez767
 */


#include <stdint.h>
#include "crc.h"

void FPU_Init(void){
	SCB_CPACR |= (0xF << 20); // FPU Calculator
}

void CRC_Init(void){
	RCC_AHB1ENR |= (1U << 12); // CRC Enable
}

uint32_t CRC_Calc(uint32_t *calc_array, uint32_t arr_length ){
	CRC_CR = (1U << 0); //Reset Control Register

	for(uint32_t i = 0; i < arr_length; i++){
		CRC_DR = calc_array[i];
	}

	return CRC_DR;
}
