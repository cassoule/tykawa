/*
 * screen.h
 *
 *  Created on: 27 avr. 2026
 *      Author: milog
 */
#include "config.h"

#ifndef SCREEN_H_
#define SCREEN_H_

void SCREEN_init(void);

void SCREEN_clear(void);

void SCREEN_draw_idle_screen(void);

void SCREEN_draw_payment_screen(uint8_t choice);

void SCREEN_draw_working_screen(uint8_t choice);

void SCREEN_draw_progress_bar(float progress);

void SCREEN_draw_done_screen(uint8_t choice);

//uint8_t SCREEN_handle_click(void);
//
//uint8_t SCREEN_handle_back_click(void);

#endif /* SCREEN_H_ */
