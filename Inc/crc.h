/*
 * crc.h
 *
 *  Created on: 2 May 2026
 *      Author: vez767
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

#define RCC_BASE		(0x40023800UL)
#define CRC_BASE		(0x40023000UL)

#define SCB_CPACR 		(*(volatile uint32_t *)(0xE000ED88UL))

#define RCC_AHB1ENR 	(*(volatile uint32_t *)(RCC_BASE + 0x30))

#define CRC_DR			(*(volatile uint32_t *)(CRC_BASE + 0x00))
#define CRC_CR			(*(volatile uint32_t *)(CRC_BASE + 0x08))

void FPU_Init(void);
void CRC_Init(void);
uint32_t CRC_Calc(uint32_t *calc_array, uint32_t arr_length );

#endif /* CRC_H_ */
