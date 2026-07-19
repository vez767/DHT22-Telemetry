/*
 * microdelay.h
 *
 *  Created on: 10 Jul 2026
 *      Author: Windows
 */

#ifndef MICRODELAY_H_
#define MICRODELAY_H_

#include <stdint.h>

#define RCC_BASE		(0x40023800UL)
#define TIM3_BASE		(0x40000400UL) // Unsigned Long

#define RCC_APB1ENR 		(*(volatile uint32_t *)(RCC_BASE + 0x40))


#define TIM3_CR1			(*(volatile uint32_t *)(TIM3_BASE + 0x00))
#define TIM3_PSC			(*(volatile uint32_t *)(TIM3_BASE + 0x28))
#define TIM3_ARR			(*(volatile uint32_t *)(TIM3_BASE + 0x2C))
#define TIM3_CNT 			(*(volatile uint32_t *)(TIM3_BASE + 0x24))
#define TIM3_EGR 			(*(volatile uint32_t *)(TIM3_BASE + 0x14))

void TIM3_Init(void);
void delay_us(uint16_t us);


#endif /* MICRODELAY_H_ */
