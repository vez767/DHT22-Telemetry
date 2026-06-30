/*
 * mpu6050.c
 *
 *  Created on: 23 Jun 2026
 *      Author: vez767
 */
#include <stdint.h>
#include "mpu6050.h"
#include "telemetry.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern SemaphoreHandle_t xI2C1_Mutex;
extern QueueHandle_t xGyroQueue;

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

/* @brief   Thread-Safe MPU Register Read Sequence
 * Requirement: Read a single 8-bit value from a specific MPU register. [MPU-REQ-004]
 * Constraint: Must utilize FreeRTOS Mutex to prevent bus collisions.
 */
uint8_t MPU6050_Read_Register(uint8_t reg_addr) {
    uint8_t received_data = 0;

    xSemaphoreTake(xI2C1_Mutex ,portMAX_DELAY);

    I2C_CR1 |= (1U << 8);
    while (!(I2C_SR1 & (1U << 0)));

    I2C_DR = MPU6050_ADDR_WRITE;
    while (!(I2C_SR1 & (1U << 1)));

    uint32_t overlook = I2C_SR1;
    overlook = I2C_SR2;
    (void)overlook;

    I2C_DR = reg_addr;
    while(!(I2C_SR1 & (1U << 7)));

    I2C_CR1 |= (1U << 8);
    while (!(I2C_SR1 & (1U << 0)));

    I2C_DR = MPU6050_ADDR_READ;
    while (!(I2C_SR1 & (1U << 1)));

    I2C_CR1 &= ~(1U << 10);

    overlook = I2C_SR1;
    overlook = I2C_SR2;
    (void)overlook;

    I2C_CR1 |= (1U << 9);

    while(!(I2C_SR1 & (1 << 6)));
    received_data = I2C_DR;

    xSemaphoreGive(xI2C1_Mutex);

    return received_data;
    }


void vMPUTask(void *pvParameters){

	MPU_Write_Register(MPU6050_PWR_MGMT_1, MPU6050_WAKEUP);

	vTaskDelay(pdMS_TO_TICKS(50));

	uint8_t gyro_h = 0; // Higher Gyroscope Value Byte
	uint8_t gyro_l = 0;	// Lower Gyroscope Value Byte

	Gryo_Payload_t mpu_data;


	while(1){

									/*X - Axis*/

		gyro_h = MPU6050_Read_Register(MPU6050_GYRO_XOUT_H);
		gyro_l = MPU6050_Read_Register(MPU6050_GYRO_XOUT_L);

		mpu_data.X_Axis = (int16_t)((gyro_h << 8) | gyro_l);
		mpu_data.X_Axis = mpu_data.X_Axis - 1930; //Calibration Offset -  X_Axis

									/*Y - Axis*/

		gyro_h = MPU6050_Read_Register(MPU6050_GYRO_YOUT_H);
		gyro_l = MPU6050_Read_Register(MPU6050_GYRO_YOUT_L);

		mpu_data.Y_Axis = (int16_t)((gyro_h << 8) | gyro_l);
		mpu_data.Y_Axis +=  1500; //Calibration Offset - Y_Axis

									/*Z - Axis*/

		gyro_h = MPU6050_Read_Register(MPU6050_GYRO_ZOUT_H);
		gyro_l = MPU6050_Read_Register(MPU6050_GYRO_ZOUT_L);

		mpu_data.Z_Axis = (int16_t)((gyro_h << 8) | gyro_l);
		mpu_data.Z_Axis -=  30; //Calibration Offset - Y_Axis

		xQueueSend(xGyroQueue, &mpu_data, pdMS_TO_TICKS(10));

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void MPU6050_Task_Init(void){
	xTaskCreate(vMPUTask, "MPU_Task", 256, NULL, 1, NULL);
}

void MPU_Write_Register(uint8_t reg_addr, uint8_t data){

	 xSemaphoreTake(xI2C1_Mutex ,portMAX_DELAY);

	  I2C_CR1 |= (1U << 8);
	  while (!(I2C_SR1 & (1U << 0)));

	  I2C_DR = MPU6050_ADDR_WRITE;
	  while (!(I2C_SR1 & (1U << 1)));

	  uint32_t overlook = I2C_SR1;
	  overlook = I2C_SR2;
	  (void)overlook;

	  I2C_DR = reg_addr;
	  while(!(I2C_SR1 & (1U << 7)));

	  I2C_DR = data;
	  while(!(I2C_SR1 & (1U << 2))); //BTF -  BYTE TRANSFER FINISHED

	  I2C_CR1 |= (1U << 9);

	  xSemaphoreGive(xI2C1_Mutex);
}
