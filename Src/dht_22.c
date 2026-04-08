#include <stdint.h>
#include "dht_22.h"


/**
 * @brief  Initializes Basic Timer 3 (TIM3) as a 1MHz microsecond stopwatch.
 * Requirement: Configure silicon to measure absolute microsecond durations for 1-Wire communication.
 * Constraint: Maximize CPU availability by preventing software-blocking delay loops; ensure the auto-reload register (ARR) ceiling is maximized to prevent early overflow.
 */


void DHT22_Start(void) {

	RCC_AHB1ENR |= (1U << 0); // GPIOA Enable

	 GPIOA_MODER &= ~(3U << 0);
	 GPIOA_MODER |= (1 << 0); // Output mode

	 GPIOA_ODR &= ~(1U << 0); // drop voltage in PA0

	 // Wait 1ms
	 Delay_us(1000);

	 GPIOA_MODER &= ~(3U << 0); // Input mode
}

 void Delay_us(uint16_t us){

	 TIM3_CNT = 0;

	 while(TIM3_CNT < us){

	 }

 }

void DHT22_Timer_Init(void){

	 RCC_APB1ENR |= (1U << 1); // TIM3 Enable

	 TIM3_ARR = 0xFFFF; // ARR Limit
	 TIM3_PSC = 15U; // Prescaler
	 TIM3_CR1 |= (1U << 0); // Enable Counter

 }

uint8_t DHT22_Check_Response(void){

	uint8_t response = 0;
	Delay_us(40);

	if(!(GPIOA_IDR & (1U << 0))){
		Delay_us(80);


	if(GPIOA_IDR & ((1U << 0))){
		response = 1;
		}
	}

	while (GPIOA_IDR & (1U << 0));

	return response;

}
uint8_t DHT22_Read_Byte(void) {

									/*This Logic for data decoding was written with the aid of Gen AI (Google Gemini, 2026)*/

    uint8_t result = 0;

    for (int i = 0; i < 8; i++) {
        // 1. Wait for the 50us LOW pulse to end
        // We wait while the pin is LOW
        while (!(GPIOA_IDR & (1 << 0)));

        // 2. The pulse has gone HIGH. Wait for (40us)
        Delay_us(40);

        // 3. Sample the pin
        if (GPIOA_IDR & (1 << 0)) {
            // If still HIGH after 40us, it's a '1'
            result |= (1 << (7 - i));
        }
        // If it's LOW, the bit is already 0, so we do nothing to 'result'

        // 4. Wait for the remainder of the HIGH pulse to finish to prevents us from misreading the next bit
        while (GPIOA_IDR & (1 << 0));
    }
    return result;
}

uint8_t data[5];

int DHT22_Get_Data(DHT22_Data_t *target) {

							/*This Logic for data validation and storage was written with the aid of Gen AI (Google Gemini, 2026)*/

    // Handshake
    DHT22_Start();
    if (DHT22_Check_Response() != 1) return -1;

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

    return 0;
}


