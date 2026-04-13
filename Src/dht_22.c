#include <stdint.h>
#include "dht_22.h"


/**
 * @brief  Initializes Basic Timer 3 (TIM3) as a 1MHz microsecond stopwatch.
 * Requirement: Configure silicon to measure absolute microsecond durations for 1-Wire communication.
 * Constraint: Maximize CPU availability by preventing software-blocking delay loops; ensure the auto-reload register (ARR) ceiling is maximized to prevent early overflow.
 */

 void delay_us(uint16_t us){

	 TIM3_CNT = 0;

	 while(TIM3_CNT < us);

 }


void DHT22_Start(void) {

	RCC_AHB1ENR |= (1U << 0); // GPIOA Enable

	GPIOA_MODER &= ~(3U << 0); // GPIO switched to Input mode to facilitate resistor pull-up

	GPIOA_PUPDR  &= ~(3U << 0); // Clear
	GPIOA_PUPDR  |= (1U << 0);
	delay_us(1000);

	 GPIOA_MODER |= (1 << 0); // Output mode
	 GPIOA_ODR &= ~(1U << 0); // drop voltage in PA0
	 delay_us(1100);

	 GPIOA_MODER &= ~(3U << 0); // Input

	 // Wait 55 microseconds for the resistor to pull the line HIGH!
	 delay_us(2);

}



void DHT22_Timer_Init(void){

	 RCC_APB1ENR |= (1U << 1); // TIM3 Enable

	 TIM3_ARR = 0xFFFF; // ARR Limit
	 TIM3_PSC = 15U; // Prescaler
	 TIM3_CR1 |= (1U << 0); // Enable Counter

 }


int8_t DHT22_Check_Response(void){
	uint16_t timeout = 0;

					/*Response from Sensor*/

	while(GPIOA_IDR & (1U << 0))
	{
		delay_us(1);
		timeout++;
		if (timeout > 2000) return -10; // Error code: No response from sensor
	}

					/*Sensor holding Line LOW(80μs)*/
	timeout = 0;
	while (!(GPIOA_IDR & (1 << 0)))
	{
	    delay_us(1);
	    timeout++;
	    if (timeout > 2000) return -11; // Error code: Sensor stuck on LOW
	}

					/*Sensor holding Line LHIGH*/
	timeout = 0;
	while (GPIOA_IDR & (1 << 0))
	{
	     delay_us(1);
	     timeout++;
	     if (timeout > 2000) return -12; // Error code: Sensor stuck on HIGH
	}

	return 1; // Success

}

					/*This Logic for data decoding was written with the aid of Generative AI (Google Gemini, 2026)*/
uint8_t DHT22_Read_Byte(void) {
			/* Clock-Independent Ratio Method */

    uint8_t result = 0;

    for (int i = 0; i < 8; i++) {
    	uint32_t low_cycles = 0;
    	uint32_t high_cycles = 0;

    	        // 1. Measure the LOW pulse
    	while (!(GPIOA_IDR & (1 << 0)))
    	{
    		low_cycles++;
    	}

    	        // 2. Measure the HIGH pulse
    	while (GPIOA_IDR & (1 << 0))
    	{
    		high_cycles++;
    	}

    	        // 3. Ratio Test (If HIGH is longer than LOW, it's a '1')
    	if (high_cycles > low_cycles)
    	{
    		result |= (1 << (7 - i));
    	}
    }

    return result;
    }

uint8_t data[5];

int8_t DHT22_Get_Data(DHT22_Data_t *target) {

							/*This Logic for data validation and storage was written with the aid of Generative AI (Google Gemini, 2026)*/

    // Handshake
    DHT22_Start();

    int8_t response = DHT22_Check_Response();
    if (response != 1) return response; // Return the exact error code (-10, -11, -12)

    // Read the 5 bytes
    for (int i = 0; i < 5; i++) {
        data[i] = DHT22_Read_Byte();
    }

    // Checksum Validation
    if (data[4] != (uint8_t)(data[0] + data[1] + data[2] + data[3])) {
        return -2; // Checksum error
    }

    // Conversion Logic
    target->Humidity = (float)((data[0] << 8) | data[1]) / 10.0; // Humidity is Byte 0 and 1 combined

    // Temperature is Byte 2 and 3 combined (Bit 15 is the sign bit)
    int16_t raw_temp = (data[2] << 8) | data[3];
    target->Temperature = (float)raw_temp / 10.0;

    return 1;
}


