/*
 * i2c_lcd.h
 *
 *  Created on: 16 Apr 2026
 *      Author: vez767
 */

#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include <stdint.h>

#define GPIOB_BASE	 		(0x40020400UL)
#define I2C1_BASE			(0x40005400UL)
#define RCC_BASE			(0x40023800UL)
#define GPIOA_BASE			(0x40020000UL)
#define TIM3_BASE			(0x40000400UL)

#define RCC_APB1ENR 		(*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_AHB1ENR 		(*(volatile uint32_t *)(RCC_BASE + 0x30))

#define GPIOB_MODER			(*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER		(*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_AFRH			(*(volatile uint32_t *)(GPIOB_BASE + 0x24))
#define GPIOB_PUPDR			(*(volatile uint32_t *)(GPIOA_BASE + 0x0C))

#define I2C_CR1 			(*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C_CR2				(*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C_CCR				(*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C_TRISE			(*(volatile uint32_t *)(I2C1_BASE + 0x20))
#define I2C_SR1				(*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C_SR2				(*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C_DR				(*(volatile uint32_t *)(I2C1_BASE + 0x10))

#define TIM3_CR1			(*(volatile uint32_t *)(TIM3_BASE + 0x00))
#define TIM3_PSC			(*(volatile uint32_t *)(TIM3_BASE + 0x28))
#define TIM3_ARR			(*(volatile uint32_t *)(TIM3_BASE + 0x2C))
#define TIM3_CNT 			(*(volatile uint32_t *)(TIM3_BASE + 0x24))
#define TIM3_EGR 			(*(volatile uint32_t *)(TIM3_BASE + 0x14))

void I2C_GPIO_Init(void);
void I2C_Config(void);
void delay_us(uint16_t us);
void TIM3_Init(void);
void I2C_ScanBus(void);
void I2C_Write(uint8_t target_address, uint8_t data);
void LCD_Send_Cmd(uint8_t cmd);
void LCD_Init(void);
void LCD_Send_Data(uint8_t data);
void LCD_Send_String(char *str);

#endif /* I2C_LCD_H_ */
