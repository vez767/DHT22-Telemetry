/*
 * mpu6050.h
 *
 *  Created on: 23 Jun 2026
 *      Author: vez767
 */

#ifndef MPU6050_H_
#define MPU6050_H_

#include <stdint.h>

#define I2C1_BASE			(0x40005400UL)
#define RCC_BASE			(0x40023800UL)
#define GPIOB_BASE	 		(0x40020400UL)

#define RCC_AHB1ENR 		(*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR 		(*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOB_MODER			(*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER		(*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_AFRH			(*(volatile uint32_t *)(GPIOB_BASE + 0x24))
#define GPIOB_PUPDR			(*(volatile uint32_t *)(GPIOB_BASE + 0x0C))

#define I2C_CR1 			(*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C_CR2				(*(volatile uint32_t *)(I2C1_BASE + 0x04))

#define I2C_SR1				(*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C_SR2				(*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C_DR				(*(volatile uint32_t *)(I2C1_BASE + 0x10))

#define MPU6050_ADDR_WRITE	0xD0
#define MPU6050_ADDR_READ   0xD1
#define MPU6050_WHO_AM_I    0x75
#define MPU6050_EXPECTED_ID 0x70

uint8_t MPU6050_Identity_Check(void);

#endif /* MPU6050_H_ */
