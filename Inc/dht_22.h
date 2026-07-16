#ifndef DHT_22_H
#define DHT_22_H

#include <stdint.h>
#include "telemetry.h"


#define RCC_BASE		(0x40023800UL)
#define GPIOA_BASE		(0x40020000UL)

#define SCB_CPACR 		(*(volatile uint32_t *)(0xE000ED88UL))

#define RCC_AHB1ENR 	(*(volatile uint32_t *)(RCC_BASE + 0x30))


#define GPIOA_MODER		(*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_IDR		(*(volatile uint32_t *)(GPIOA_BASE + 0x10))
#define GPIOA_ODR		(*(volatile uint32_t *)(GPIOA_BASE + 0x14))
#define GPIOA_PUPDR		(*(volatile uint32_t *)(GPIOA_BASE + 0x0C))


void delay_us(uint16_t us);
void DHT22_Start(void);
int8_t DHT22_Check_Response(void);
uint8_t DHT22_Read_Byte(void);
void DHT22_Task_Init(void);
void vClimateTask(void *pvParameters);
int8_t DHT22_Get_Data(Climate_Payload_t *target);

#endif
