#ifndef DHT_22_H
#define DHT_22_H

#define TIM3_BASE		(0x40000400UL) // Unsigned Long
#define RCC_BASE		(0x40023800UL)
#define GPIOA_BASE		(0x40020000UL)

#define RCC_APB1ENR 	(*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_AHB1ENR 	(*(volatile uint32_t *)(RCC_BASE + 0x30))

#define TIM3_CR1		(*(volatile uint32_t *)(TIM3_BASE + 0x00))
#define TIM3_PSC		(*(volatile uint32_t *)(TIM3_BASE + 0x28))
#define TIM3_ARR		(*(volatile uint32_t *)(TIM3_BASE + 0x2C))
#define TIM3_CNT 		(*(volatile uint32_t *)(TIM3_BASE + 0x24))

#define GPIOA_MODER		(*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_IDR		(*(volatile uint32_t *)(GPIOA_BASE + 0x10))
#define GPIOA_ODR		(*(volatile uint32_t *)(GPIOA_BASE + 0x14))

#endif
