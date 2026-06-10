/*
 * iwdg.c
 *
 *  Created on: 27 Apr 2026
 *      Author: vez767
 */

#include "iwdg.h"

 // Requirement: Initialize Independent Watchdog (IWDG) for a 2-second timeout.
 // Constraint: Must survive main system clock failure by utilizing the 32kHz LSI clock.
void IWDG_Init(void){
	IWDG_KR = 0xCCCCU; // Turn on Watchdog Key Register

	IWDG_KR = 0x5555U; // Unlock

	IWDG_PR = 0x3U; // - Divider /32

	IWDG_RLR = 4000U;
	while(IWDG_SR & (0x03U << 0));

	IWDG_KR = 0xAAAAU;
}
