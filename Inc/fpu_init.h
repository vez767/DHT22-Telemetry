/*
 * fpu_init.h
 *
 *  Created on: 12 May 2026
 *      Author: Windows
 */

#ifndef FPU_INIT_H_
#define FPU_INIT_H_

#include <stdint.h>

#define SCB_CPACR 		(*(volatile uint32_t *)(0xE000ED88UL))

void FPU_Init(void);

#endif /* FPU_INIT_H_ */
