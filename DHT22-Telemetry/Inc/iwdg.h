/*
 * iwdg.h
 *
 *  Created on: 27 Apr 2026
 *      Author: vez767
 */

#ifndef IWDG_H_
#define IWDG_H_

#include <stdint.h>

#define IWDG_BASE 			(0x40003000UL)

#define IWDG_KR 		(*(volatile uint32_t *)(IWDG_BASE + 0x00))
#define IWDG_PR 		(*(volatile uint32_t *)(IWDG_BASE + 0x04))
#define IWDG_RLR 		(*(volatile uint32_t *)(IWDG_BASE + 0x08))
#define IWDG_SR 		(*(volatile uint32_t *)(IWDG_BASE + 0x0C))

void IWDG_Init(void);

#endif /* IWDG_H_ */
