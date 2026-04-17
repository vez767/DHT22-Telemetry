/*
 * i2c_lcd.c
 *
 *  Created on: 16 Apr 2026
 *      Author: vez767
 */

#include <stdint.h>
#include "i2c_lcd.h"
#include <stdio.h>


void I2C_GPIO_Init(void){

	RCC_AHB1ENR |= (1 << 1);
	GPIOB_MODER	&= ~((3U << 16) | (3U << 18));
	GPIOB_MODER	|= ((2U << 16) | (2U << 18)); // Alternate Function

	GPIOB_OTYPER |= (3U << 8); // Open drain Mode

	GPIOB_AFRH &= ~(0xFFU << 0); // Clear pins 8 & 9
	GPIOB_AFRH |= (0x44U << 0); // AF4 (0100)
}

void I2C_Config(void){

	RCC_APB1ENR |= (1 << 21); // Enable I2C1

	I2C_CR1 &= ~(1 << 0); // Clear Peripheral Enable

	I2C_CR2 &= ~(0x1F << 0); // Clear
	I2C_CR2 = (16U << 0); // Set Frequency(FREQ)

	I2C_CCR = (80U << 0); // Formula: FREQ/(2 * 100 000)
	I2C_TRISE = (17U << 0); // Formula: (1000/(10^9/FREQ * 1 000 000)) + 1   	[MHz = 1 000 000]

	I2C_CR1 |= (1 << 0); // Turn on PE (Peripheral Enable)

}

void Delay_us(uint16_t us){

	TIM3_CNT = 0;

	while(TIM3_CNT < us);

}

void TIM3_Init(void){

	 RCC_APB1ENR |= (1U << 1); // TIM3 Enable

	 TIM3_ARR = 0xFFFF; // ARR Limit
	 TIM3_PSC = 15U; // Prescaler
	 TIM3_EGR |= (1U <<0);
	 TIM3_CR1 |= (1U << 0); // Enable Counter

 }



							/*	THIS FUNCTION WAS WRITTEN WITH THE AID OF GENERATIVE AI.
							 	A PROMPT WAS GIVEN FOR ASSIGNMENT OF TASKS AT CURRENT LEVEL
							 	OF UNDERSTANDING TO AID COMPREHENSION AND FORSTER KNOWLEDGE	*/
void I2C_ScanBus(void) {
    printf("Starting I2C Scan...\n");

    for (uint8_t i = 0; i < 128; i++) {

        // 1. Generate START
        // YOUR TASK: Set Bit 8 in I2C_CR1
    	I2C_CR1 |= (1 << 8);

        // 2. Wait for START to finish
        // YOUR TASK: while loop waiting for Bit 0 in I2C_SR1 to become 1
    	while(!(I2C_SR1 & (1 << 0)));

        // 3. Send the Address (Shifted by 1 for the R/W bit)
        I2C_DR = (i << 1);

        // 4. Wait for ADDR (Success) OR AF (Failure)
        while (!(I2C_SR1 & (1 << 1)) && !(I2C_SR1 & (1 << 10))) {
            // Wait until either ADDR (Bit 1) or AF (Bit 10) goes to 1
        }

        // 5. Process the Reply
        if (I2C_SR1 & (1 << 1)) { // If ADDR is 1 (Success!)
            printf("Found device at address: 0x%X\n", i);

            // Clear ADDR flag by reading SR1 then SR2
            uint32_t temp = I2C_SR1;
            temp = I2C_SR2;
            (void)temp;
        }
        else { // If AF is 1 (Nobody answered)
            // YOUR TASK: Clear the AF flag by clearing Bit 10 in I2C_SR1
        	I2C_SR1 &= ~(1U << 10);
        }

        // 6. Generate STOP
        // YOUR TASK: Set Bit 9 in I2C_CR1
        	I2C_CR1 |= (1U << 9);

        Delay_us(2000); // Short breather before checking the next address
    }
    printf("Scan Complete.\n");
}
