/*
 * hcrs04.c
 *
 *  Created on: 7 Jul 2026
 *      Author: vez767
 */

#include <stdint.h>
#include <stdlib.h>
#include "hcsr04.h"
#include "microdelay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define ABS_DIFF(a, b) ( (int32_t)(a) > (int32_t)(b) ? ((int32_t)(a) - (int32_t)(b)) : ((int32_t)(b) - (int32_t)(a)) )


void HCSR04_Init(void){

	RCC_AHB1ENR |= (1U << 0); // GPIOA-EN

	//PINS: D8(PA9) - OUTPUT & PA1 - ALTERNATE FUNCTION
	GPIOA_MODER &= ~((3U << 2) | (3U << 18));
	GPIOA_MODER |= ((2U << 2) | (1U << 18));

	GPIOA_AFRL &= ~(0xFU << 4); // AFRL1[3:0]
	GPIOA_AFRL |= (1U << 4);

					/*TIM2 Config*/
	RCC_APB1ENR |= (1U << 0);

	TIM2_PSC = 15U;
	TIM2_ARR = 0xFFFFFFFF;
	TIM2_EGR |= (1 << 0);

	// ECHO Logic
	TIM2_CCMR1 &= ~(3U << 8);
	TIM2_CCMR1 |= (1U << 8); //  Capture/Compare 2 selection: Input - TI2

	TIM2_CCER |= ((1U << 5) | (1U << 7) | (1U << 4)); // CC2P/CC2NP: [11] - Sensitive to Both Edges AND CC2E - EN

	TIM2_DIER |= (1U << 2); // CC2IE: Capture/Compare 2 interrupt enable

	TIM2_CR1 |= (1 << 0);

	NVIC_IPR7 &= ~(0xFFU << 0);
	NVIC_IPR7 |= ((5U << 4) << 0);

	NVIC_ISER0 |= (1U << 28);
}

extern TaskHandle_t xHCSR04TaskHandle;
void TIM2_IRQHandler(void){

	static volatile uint32_t capture_start = 0;
	static volatile uint32_t capture_end = 0;
	uint32_t capture_duration = 0;

	if(TIM2_SR & (1U << 2)){

		if(GPIOA_IDR & (1U << 1)){
			// Rising Edge
			capture_start = TIM2_CCR2;

		}else{
				// Falling Edge
			 capture_end = TIM2_CCR2;
			 capture_duration = capture_end - capture_start;

			 BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			 xTaskNotifyFromISR(xHCSR04TaskHandle, capture_duration, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
			 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}

		TIM2_SR &= ~(1U << 2); // Clear Flag
	}
}



extern QueueHandle_t xDistanceQueue;

void vHCSR04_Task(void *pvParameters) {
	uint32_t capture_duration = 0;
    uint32_t distance_cm = 0;

    uint32_t dist_buffer[10] = {0};
    uint8_t dist_buffer_index = 0;
    uint32_t last_valid_dist = 0;
    uint32_t value_to_send = 0;
    uint32_t last_value_sent = 9999;
    uint8_t is_first_boot = 1;
    uint8_t anomaly_count = 0;

    while(1) {

        GPIOA_BSRR = (1U << 9);          // PA9 - HIGH
        delay_us(13);
        GPIOA_BSRR = (1U << (9 + 16));   // PA9 - LOW

      if(xTaskNotifyWait(0x00, 0xFFFFFFFF, &capture_duration, pdMS_TO_TICKS(50)) == pdTRUE){

        // SAFETY CHECK
        if (capture_duration > 0 && capture_duration < 38000) { // 38000us is roughly 6.5 meters; past sensor limit

            distance_cm = capture_duration / 58;
        } else {

            distance_cm = 999; // Error Code
        }

        capture_duration = 0;


        if(distance_cm != 999){

        	if(is_first_boot == 1 || ABS_DIFF(distance_cm, last_valid_dist ) <= 20 || anomaly_count >= 3){

        		if (is_first_boot == 1 || anomaly_count >= 3) {
                    for (uint8_t i = 0; i < 10; i++) {
                        dist_buffer[i] = distance_cm;
                    }
                    is_first_boot = 0;
                }else {

                    dist_buffer[dist_buffer_index] = distance_cm;
                    dist_buffer_index = (dist_buffer_index + 1) % 10;
                }

        	last_valid_dist = distance_cm;
        	anomaly_count = 0;

        	uint32_t cummulative_distance = 0;

        	for (uint8_t i = 0; i < 10; i++) {

        		cummulative_distance += dist_buffer[i];
        	}

        	value_to_send = (uint32_t)(cummulative_distance / 10);

        	}else{
        		anomaly_count++;
        	}

        }else {

    		value_to_send = 999; // Error Code
    		is_first_boot = 1; // Sensor Fault
        }

      }else value_to_send = 888; // Error Code
      is_first_boot = 1;

        if (xDistanceQueue != NULL && last_value_sent != value_to_send) {
            xQueueSend(xDistanceQueue, &value_to_send, 0);
            last_value_sent = value_to_send;
        }
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void HCSR04_Task_Init(void){
	xTaskCreate(vHCSR04_Task , "vHCSR04_Task", 256, NULL, 3, &xHCSR04TaskHandle);
}
