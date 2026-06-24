/*
 * mpu6050.c
 *
 *  Created on: 23 Jun 2026
 *      Author: vez767
 */
#include <stdint.h>
#include "mpu6050.h"


uint8_t MPU6050_Identity_Check(void){

	uint8_t MPU6050_ID = 0;

	I2C_CR1 |= (1U << 8);
	while (!(I2C_SR1 & (1U << 0))); // Start Bit (SB)

	I2C_DR = MPU6050_ADDR_WRITE;	// MPU Identification Register Address (0x68 << 0) - WRITE
	while (!(I2C_SR1 & (1U << 1))); // ADDR

	uint32_t overlook = I2C_SR1;
	overlook = I2C_SR2;
	(void)overlook;

	while(!(I2C_SR1 & (1U << 7))); // TXE
	I2C_DR = MPU6050_WHO_AM_I;		// (0x68 << 1) - READ
	while(!(I2C_SR1 & (1U << 7)));

	// REPEATED START
	I2C_CR1 |= (1U << 8);
	while (!(I2C_SR1 & (1U << 0)));

	I2C_DR = MPU6050_ADDR_READ;
	while (!(I2C_SR1 & (1U << 1)));

	I2C_CR1 &= ~(1U << 10); // Clear Automaic ACK bit

	overlook = I2C_SR1;
	overlook = I2C_SR2;
	(void)overlook;

	I2C_CR1 |= (1U << 9); // STOP - Stop after the current/1st Byte transfer.

	while(!(I2C_SR1 & (1 << 6))); // RxNE - Data Register (Receiver) is not empty.
	MPU6050_ID = I2C_DR;

	return MPU6050_ID;
}

