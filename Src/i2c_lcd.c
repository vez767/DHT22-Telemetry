/*
 * i2c_lcd.c
 *
 *  Modified on: 14 June 2026
 *      Author: vez767
 */

#include <stdint.h>
#include "i2c_lcd.h"
#include "telemetry.h"
#include "microdelay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"

extern SemaphoreHandle_t xI2C1_Mutex;


void I2C_GPIO_Init(void){

	RCC_AHB1ENR |= (1 << 1);
	GPIOB_MODER	&= ~((3U << 16) | (3U << 18));
	GPIOB_MODER	|= ((2U << 16) | (2U << 18)); // Alternate Function

	GPIOB_OTYPER |= (3U << 8); // Open drain Mode

	GPIOB_AFRH &= ~(0xFFU << 0); // Clear pins 8 & 9
	GPIOB_AFRH |= (0x44U << 0); // AF4 (0100)

	GPIOB_PUPDR	&= ~(0xFU << 16);
	GPIOB_PUPDR	|= (5U << 16); // Pull-Up Mode (01) - pins 8 & 9
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


							/*	THIS FUNCTION WAS WRITTEN WITH THE AID OF GENERATIVE AI (Google Gemini, 2026).
							 	A PROMPT WAS GIVEN FOR ASSIGNMENT OF TASKS AT CURRENT LEVEL
							 	OF UNDERSTANDING TO AID COMPREHENSION AND FORSTER KNOWLEDGE	*/
 void I2C_ScanBus(void) {

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

        vTaskDelay(pdMS_TO_TICKS(2)); // Short breather before checking the next address
    }

}

/* @brief 	Implementation logic for foundational I2C data transmission.
 * Requirement: Transmit a single 8-bit payload to a specific data address.
 * Constraints: CPU must wait for respective hardware flags before proceeding.
 */



void I2C_Write(uint8_t target_address, uint8_t data) {

	xSemaphoreTake(xI2C1_Mutex, portMAX_DELAY);
	I2C_CR1 |= (1 << 8); //Start bit
	while (!(I2C_SR1 & (1 << 0)));

	I2C_DR = (target_address << 1);
	while (!(I2C_SR1 & (1 << 1))); //Wait for ADDR flag

	uint32_t temp = I2C_SR1;
	temp = I2C_SR2;
	(void)temp;

	while(!(I2C_SR1 & (1 << 7))); // Wait for TxE(Data register empty) flag
	I2C_DR = data; //Load data into DR

	while(!(I2C_SR1 & (1 << 2))); // BTF(Byte Transfer Finished) flag

	I2C_CR1 |= (1 << 9);
	xSemaphoreGive(xI2C1_Mutex);

}

/* @brief	Logic for LCD state and character format instructions.
 * Requirement: Transmit an 8-bit command to the HD44780 controller in 4-bit mode.
 * Constraint: Must pulse the EN pin for both the upper and lower nibbles to capture sent data.
 */


											/*LCD STATE LAYOUT*/

		/*						The LCD operates by 8-bit parallel wire logic (XXXX - X - X - X - X)
				Bits 4-8	/	 Bit 3		/	 Bit 2		/	 Bit 1		 /		Bit 0
					|		\	 	|		\		|		\		|		 \		   |
				DATA PINS	/	Backlight	/	 Enable		/	Read/Write	 /	 RS[0(instruction)/1(data)]
		*/

void LCD_Send_Cmd(uint8_t target_address, uint8_t cmd){
	uint8_t upper_nibble = (cmd & 0xF0U);
	uint8_t lower_nibble = ((cmd << 4) & 0xF0U);

	uint8_t backlight = 0x08U; // LCD backlight
	uint8_t enable = 0x04U;// Enable pin (Bit 2)

							//Pulse Upper Nibble(UN)
	uint8_t UN_enable_high = upper_nibble | backlight | enable;
	I2C_Write(target_address, UN_enable_high);
	delay_us(1);

	uint8_t UN_enable_low = upper_nibble | backlight;
	I2C_Write(target_address, UN_enable_low);
	delay_us(50);

							//Pulse Lower Nibble(LN)
	uint8_t LN_enable_high = lower_nibble | backlight | enable;
	I2C_Write(target_address, LN_enable_high);
	delay_us(1);

	uint8_t LN_enable_low = lower_nibble | backlight;
	I2C_Write(target_address, LN_enable_low);
	delay_us(50);

}

//@brief	Implement LCD initalization sequence.
// Requirement: Set  HD44780 controller into 4-bit mode.
// Constraint: Must adhere to the manufacturer's strict timing sequence.


						/*	THIS FUNCTION WAS WRITTEN WITH THE AID OF GENERATIVE AI (Google Gemini, 2026).
							 A PROMPT WAS GIVEN FOR ASSIGNMENT OF TASKS AT CURRENT LEVEL
							 OF UNDERSTANDING TO AID COMPREHENSION AND FORSTER KNOWLEDGE	*/
void LCD_Init(uint8_t target_address) {
	// 1. Wait for power to stabilize
	vTaskDelay(pdMS_TO_TICKS(53));

	// 2. The specific wake-up sequence
	LCD_Send_Cmd(target_address, 0x30);
	vTaskDelay(pdMS_TO_TICKS(5));

	LCD_Send_Cmd(target_address, 0x30);
	vTaskDelay(pdMS_TO_TICKS(2));

	LCD_Send_Cmd(target_address, 0x30);
	vTaskDelay(pdMS_TO_TICKS(10));

	// 3. Lock into 4-bit mode
	LCD_Send_Cmd(target_address, 0x20);
	vTaskDelay(pdMS_TO_TICKS(10));

	    // ----------------------------------------------------
	    // YOUR TASK: Finish the configuration (Steps 6 through 10)
	    // ----------------------------------------------------

	    // Step 6: 2 lines, 5x8 font (Hex: 0x28)
	LCD_Send_Cmd(target_address, 0x28);
	vTaskDelay(pdMS_TO_TICKS(2));

	    // Step 7: Display OFF (Hex: 0x08)
	LCD_Send_Cmd(target_address, 0x08);
	vTaskDelay(pdMS_TO_TICKS(2));

	    // Step 8: Clear Display (Hex: 0x01) -> Needs a 2000us delay
	LCD_Send_Cmd(target_address, 0x01);
	vTaskDelay(pdMS_TO_TICKS(2));

	    // Step 9: Entry Mode Set, Left-to-right (Hex: 0x06)
	LCD_Send_Cmd(target_address, 0x06);
	vTaskDelay(pdMS_TO_TICKS(2));

	    // Step 10: Display ON, Cursor OFF (Hex: 0x0C)
	LCD_Send_Cmd(target_address, 0x0C);
	vTaskDelay(pdMS_TO_TICKS(2));
	}


void LCD_Send_Data(uint8_t target_address ,uint8_t data){
	uint8_t upper_nibble = (data & 0xF0U);
	uint8_t lower_nibble = ((data << 4) & 0xF0U);

	uint8_t backlight = 0x08U;
	uint8_t enable = 0x04U;
	uint8_t RS = 1U;

							//Pulse Upper Nibble(UN)
	uint8_t UN_enable_high = upper_nibble | backlight |	RS | enable;
	I2C_Write(target_address, UN_enable_high);
	delay_us(1);

	uint8_t UN_enable_low = upper_nibble | backlight |	RS ;
	I2C_Write(target_address, UN_enable_low);
	delay_us(50);

							//Pulse Lower Nibble(LN)
	uint8_t LN_enable_high = lower_nibble | backlight |	RS | enable;
	I2C_Write(target_address, LN_enable_high);
	delay_us(1);

	uint8_t LN_enable_low = lower_nibble | backlight |	RS;
	I2C_Write(target_address, LN_enable_low);
	delay_us(50);

}
//@brief Programmatically sends a character sequence to the LCD until a null terminator is reached
// Requirement: Transmit a null-terminated string to the LCD.
// Constraint: Must utilize the existing LCD_Send_Data wrapper.

void LCD_Send_String(uint8_t target_address, char *str) {

	while(*str != '\0'){
		LCD_Send_Data(target_address, *str++);
	}
}

// Requirement: Convert a positive integer into a null-terminated ASCII string.
// Constraint: Do not use <stdio.h> or standard libraries.
void Int_To_String(uint32_t num, char *str) {

	int32_t i = 0;

	if(num == 0) {
	   str[i++] = '0'; // Store the character '0' and move the index
	   str[i] = '\0';  // Null-terminate
	   return;
	}

	while(num > 0){
		str[i++] = (num % 10) + 0x30;
		num /= 10;
	}
	str[i] = '\0';

	int32_t incremental_index = 0;
	int32_t decremental_index = i - 1;
		char temp;

		while (incremental_index < decremental_index) {
		    temp = str[incremental_index];
		    str[incremental_index] = str[decremental_index];
		    str[decremental_index] = temp;
		    incremental_index++;
		    decremental_index--;
		}

}

						 /*  THIS FUNCTION WAS WRITTEN WITH THE AID OF GENERATIVE AI (Google Gemini, 2026).
							 A PROMPT WAS GIVEN FOR ASSIGNMENT OF TASKS AT CURRENT LEVEL
							 OF UNDERSTANDING TO AID COMPREHENSION AND FOSTER KNOWLEDGE	*/

void Float_To_String(float num, char *str) {

    // 1. Extract the whole number (e.g., 24.5 becomes 24)
    int32_t whole_part = (int32_t)num;

    // 2. Extract the fraction and turn it into a whole number (e.g., 24.5 - 24.0 = 0.5 * 10 = 5)
    float fraction_part = num - (float)whole_part;
    int32_t fraction_int = (int32_t)(fraction_part * 10.0f);

    // Safety check for negative fractions
    if(fraction_int < 0) {
        fraction_int = -fraction_int;
    }

    // YOUR TASK:
    // 3. Send 'whole_part' and the 'str' array to your Int_To_String engine
    Int_To_String(whole_part, str);


    // 4. Find the end of the string (the '\0') so we know where to put the decimal
    uint32_t i = 0;
    while(str[i] != '\0') {
        i++;
    }

    // 5. Overwrite the '\0' with a '.' and increment 'i' by 1
    str[i++] = '.';


    // 6. Send the 'fraction_int' to your Int_To_String engine.
    // HINT: Pass the address of the exact slot where the fraction should start using &str[i]
    Int_To_String(fraction_int, &str[i]);
}

// Requirement: Move the internal LCD DDRAM cursor to a specific row and column.
// Constraint: Row must be 0 or 1. Column must be bounded to 0-15 to prevent off-screen memory writes.

void LCD_Set_Cursor(uint8_t target_address, uint8_t row, uint8_t col) {
    uint8_t cursor_cmd;

    if (row == 0) {
        cursor_cmd = 0x80U + col;        // Base address for Row 0 - 0x80.
    } else {
        cursor_cmd = 0xC0U + col;		// Base address for Row 1 - 0xC0.
    }


    LCD_Send_Cmd(target_address, cursor_cmd);
}


void Pad_String(char* buffer, uint8_t target_length){
	uint8_t i = 0;
	while(buffer[i] != '\0'){
		i++;
	}

	while(i < target_length){
	        buffer[i] = ' ';
	        i++;
	}

	buffer[i] = '\0';
}


extern QueueHandle_t xClimateQueue;
extern QueueHandle_t xGyroQueue;
extern QueueHandle_t xDistanceQueue;

volatile UBaseType_t display_watermark = 0;

void vDisplayTask(void *pvParameters){

	Climate_Payload_t Received_Data;
	Climate_Payload_t Displayed_Data;
	Gryo_Payload_t Received_Gyro;
	Acoustic_Payload_t HCSR04;

	char temp_string_box[16];
    char hum_string_box[16];
    char gyro_string_box[16];
    char dist_string_box[16];

    Displayed_Data.Temperature = 0.0f;
	Displayed_Data.Humidity = 0.0f;


    LCD_Init(ENV_DISP_ADDRESS);
    vTaskDelay(pdMS_TO_TICKS(3));

    LCD_Init(NAV_DISP_ADDRESS);
    vTaskDelay(pdMS_TO_TICKS(3));

    LCD_Init(DISTANCE_DISP_ADDRESS);
    vTaskDelay(pdMS_TO_TICKS(3));

    LCD_Send_Cmd(ENV_DISP_ADDRESS, 0x01);
    LCD_Send_Cmd(NAV_DISP_ADDRESS, 0x01);
    LCD_Send_Cmd(DISTANCE_DISP_ADDRESS, 0x01);
    vTaskDelay(pdMS_TO_TICKS(5));

    // Environment Screen (ENV_DISP_ADDRESS)
    LCD_Set_Cursor(ENV_DISP_ADDRESS, 0, 0);
    LCD_Send_String(ENV_DISP_ADDRESS, "TEMP:       C");
    LCD_Set_Cursor(ENV_DISP_ADDRESS, 1, 0);
    LCD_Send_String(ENV_DISP_ADDRESS, "HUM:        %");

    // Navigation Screen (NAV_DISP_ADDRESS)
    LCD_Set_Cursor(NAV_DISP_ADDRESS, 0, 0);
    LCD_Send_String(NAV_DISP_ADDRESS, "X:       Y: ");
    LCD_Set_Cursor(NAV_DISP_ADDRESS, 1, 0);
    LCD_Send_String(NAV_DISP_ADDRESS, "Z: ");

    // Distance Screen (DISTANCE_DISP_ADDRESS)
    LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 0, 0);
    LCD_Send_String(DISTANCE_DISP_ADDRESS, "DIST: ");
    LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 0, 14); // Setup for the 'cm' unit
    LCD_Send_String(DISTANCE_DISP_ADDRESS, "cm");

    LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 1, 0);
    LCD_Send_String(DISTANCE_DISP_ADDRESS, "STATUS: ");

    while(1) {
    										/* DHT-22 */

    	if (xQueueReceive(xClimateQueue, &Received_Data, pdMS_TO_TICKS(10)) == pdPASS) {
    	    uint8_t redraw_needed = 0;

    	    							/*	HYSTERESIS FILTER	*/
    	    if (Received_Data.Sensor_Status == 1) {

    	    	float temp_diff = Received_Data.Temperature - Displayed_Data.Temperature;
    	    	if (temp_diff < 0.0f) temp_diff = -temp_diff;

    	    	float hum_diff = Received_Data.Humidity - Displayed_Data.Humidity;
    	    	if (hum_diff < 0.0f) hum_diff = -hum_diff;


    	    	if(temp_diff >= 0.5f || hum_diff >= 1.0f) {

    	    		Displayed_Data.Temperature = Received_Data.Temperature;
    	    		Displayed_Data.Humidity = Received_Data.Humidity;

    	    		redraw_needed = 1;
    	    	}

    	    }else{
    	    	 // HARDWARE FAULT: Overwrite the payload with diagnostic codes.

    	    	Displayed_Data.Temperature = 999.0f;
    	    	Displayed_Data.Humidity = 999.0f;
    	    	redraw_needed = 1;
    	    }

    	    	/*	Logic to save CPU cycles for only when character redrawing is needed	*/

    	    if (redraw_needed == 1){

    	    	Float_To_String(Displayed_Data.Temperature, temp_string_box);
    	    	Float_To_String(Displayed_Data.Humidity, hum_string_box);

    	    	Pad_String(temp_string_box, 6);
    	    	Pad_String(hum_string_box, 6);

    	    	LCD_Set_Cursor(ENV_DISP_ADDRESS, 0, 6);
    	    	LCD_Send_String(ENV_DISP_ADDRESS, temp_string_box);

    	   	    LCD_Set_Cursor(ENV_DISP_ADDRESS, 1, 6);
     		    LCD_Send_String(ENV_DISP_ADDRESS, hum_string_box);

  	    	}
    	}
    						/* GYRO */

    	if(xQueueReceive(xGyroQueue, &Received_Gyro, pdMS_TO_TICKS(10)) == pdPASS){

    		if (Received_Gyro.X_Axis > -70 && Received_Gyro.X_Axis < 70) Received_Gyro.X_Axis = 0;
    		if (Received_Gyro.Y_Axis > -70 && Received_Gyro.Y_Axis < 70) Received_Gyro.Y_Axis = 0;
    		if (Received_Gyro.Z_Axis > -70 && Received_Gyro.Z_Axis < 70) Received_Gyro.Z_Axis = 0;

    		uint32_t gyro_reading = 0;

    // X-Axis ---->

    		LCD_Set_Cursor(NAV_DISP_ADDRESS, 0, 2);
    		if (Received_Gyro.X_Axis < 0){

    			LCD_Send_String(NAV_DISP_ADDRESS, "-");
    			gyro_reading = (uint32_t)(Received_Gyro.X_Axis * -1);

    		}else {
    			LCD_Send_String(NAV_DISP_ADDRESS, "+");
    			gyro_reading = (uint32_t)Received_Gyro.X_Axis;
    		}
    		    Int_To_String(gyro_reading, gyro_string_box);
    		    Pad_String(gyro_string_box, 5);
    		    LCD_Send_String(NAV_DISP_ADDRESS, gyro_string_box);

    // <---

    // Y-Axis ---->
    		 LCD_Set_Cursor(NAV_DISP_ADDRESS, 0, 11);
    		 if (Received_Gyro.Y_Axis < 0){

    		     LCD_Send_String(NAV_DISP_ADDRESS, "-");
    		     gyro_reading = (uint32_t)(Received_Gyro.Y_Axis * -1);

    		  }else {

    		     LCD_Send_String(NAV_DISP_ADDRESS, "+");
    		     gyro_reading = (uint32_t)Received_Gyro.Y_Axis;
    		  }
    		     Int_To_String(gyro_reading, gyro_string_box);
    		     Pad_String(gyro_string_box, 5);
    		     LCD_Send_String(NAV_DISP_ADDRESS, gyro_string_box);

    // <----

    // Z-Axis ---->

    		  LCD_Set_Cursor(NAV_DISP_ADDRESS, 1, 2);
    		  if (Received_Gyro.Z_Axis < 0){

    			  LCD_Send_String(NAV_DISP_ADDRESS, "-");
    		      gyro_reading = (uint32_t)(Received_Gyro.Z_Axis * -1);

    		  }else {

    		      LCD_Send_String(NAV_DISP_ADDRESS, "+");
    		      gyro_reading = (uint32_t)Received_Gyro.Z_Axis;
    		  }
    		      Int_To_String(gyro_reading, gyro_string_box);
    		      Pad_String(gyro_string_box, 5);
    		      LCD_Send_String(NAV_DISP_ADDRESS, gyro_string_box);

    // <----


    	}

    							/* DISTANCE */
    	if(xQueueReceive(xDistanceQueue, &HCSR04, pdMS_TO_TICKS(10)) == pdPASS){

    		LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 0, 6);

    		if(HCSR04.status == 1){ //ERR CODE 1: SUCCESS

    			Int_To_String(HCSR04.distance, dist_string_box);
    			Pad_String(dist_string_box, 8);
    			LCD_Send_String(DISTANCE_DISP_ADDRESS, dist_string_box);

    			LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 0, 14);
    			LCD_Send_String(DISTANCE_DISP_ADDRESS, "cm");

    			LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 1, 8);
    			LCD_Send_String(DISTANCE_DISP_ADDRESS, " 1 ");

    		}else if((HCSR04.status == -1)){	//ERR CODE -1: TARGET LOST

    			LCD_Send_String(DISTANCE_DISP_ADDRESS, "TRGT LOST");

    			LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 1, 8);
    			LCD_Send_String(DISTANCE_DISP_ADDRESS, "-1 ");

    		}else{ //ERR CODE -2: HARDWARE FAULT(WIRING)
    			LCD_Send_String(DISTANCE_DISP_ADDRESS, "WIRE FAULT");

    			LCD_Set_Cursor(DISTANCE_DISP_ADDRESS, 1, 8);
    			LCD_Send_String(DISTANCE_DISP_ADDRESS, "-2 ");
    		}

    	}


    	vTaskDelay(pdMS_TO_TICKS(10));
        display_watermark = uxTaskGetStackHighWaterMark(NULL);
    }
}

void LCD_Task_Init(void) {
    // Priority 2 ensures the screen draws data immediately when available
    xTaskCreate(vDisplayTask, "Display", 256, NULL, 2, NULL);
}



