/*
 * fpu_init.c
 *
 *  Created on: 12 May 2026
 *      Author: vez767
 */
#include <stdint.h>
#include "fpu_init.h"

void FPU_Init(void){
	SCB_CPACR |= (0xF << 20); // FPU Calculator
}

