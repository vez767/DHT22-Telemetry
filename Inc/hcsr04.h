/*
 * hcrs04.h
 *
 *  Created on: 7 Jul 2026
 *      Author: vez767
 */

#ifndef HCSR04_H_
#define HCSR04_H_

#include <stdint.h>

#define RCC_BASE		(0x40023800UL)
#define GPIOA_BASE		(0x40020000UL)
#define TIM2_BASE		(0x40000000UL)
#define NVIC_IPR_BASE   (0xE000E400UL)
#define NVIC_ISER_BASE	(0xE000E100UL)

#define RCC_APB1ENR 	(*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_AHB1ENR 	(*(volatile uint32_t *)(RCC_BASE + 0x30))


#define GPIOA_MODER		(*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL		(*(volatile uint32_t *)(GPIOA_BASE + 0x20))
#define GPIOA_IDR		(*(volatile uint32_t *)(GPIOA_BASE + 0x10))
#define GPIOA_BSRR		(*(volatile uint32_t *)(GPIOA_BASE + 0x18))


#define TIM2_EGR		(*(volatile uint32_t *)(TIM2_BASE + 0x14))
#define TIM2_PSC		(*(volatile uint32_t *)(TIM2_BASE + 0x28))
#define TIM2_CNT		(*(volatile uint32_t *)(TIM2_BASE + 0x24))
#define TIM2_ARR		(*(volatile uint32_t *)(TIM2_BASE + 0x2C))
#define TIM2_CR1		(*(volatile uint32_t *)(TIM2_BASE + 0x00))
#define TIM2_CCMR1		(*(volatile uint32_t *)(TIM2_BASE + 0x18))
#define TIM2_CCER		(*(volatile uint32_t *)(TIM2_BASE + 0x20))
#define TIM2_DIER		(*(volatile uint32_t *)(TIM2_BASE + 0x0C))
#define TIM2_SR			(*(volatile uint32_t *)(TIM2_BASE + 0x10))
#define TIM2_CCR2		(*(volatile uint32_t *)(TIM2_BASE + 0x38))


#define NVIC_IPR7		(*(volatile uint32_t *)(NVIC_IPR_BASE + 0x1C))
#define NVIC_ISER0 		(*(volatile uint32_t *)(NVIC_ISER_BASE + 0x00))

void HCSR04_Init(void);
void TIM2_IRQHandler(void);
void vHCSR04_Task(void *pvParameters);
void HCSR04_Task_Init(void);


#endif /* HCSR04_H_ */
