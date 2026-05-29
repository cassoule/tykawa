/*
 * servo.c
 *
 *  Created on: 27 avr. 2026
 *      Author: milog
 */
#include "config.h"
#include "stm32g4_timer.h"
#include "stm32g4_utils.h"
#include "servo.h"

#define PERIOD_TIMER 10

static uint16_t current_position;
uint16_t position;

/**
 * @brief  Initialise le servomoteur
 *
 */
void SERVO_init(void) {
	BSP_TIMER_run_us(TIMER1_ID, PERIOD_TIMER*1000, false);

	BSP_TIMER_enable_PWM(TIMER1_ID, TIM_CHANNEL_3, 150, false, false);

	SERVO_set_position(0);
}

/**
 * @brief  Positionne le servo moteur
 *
 * @param position: position souhaitée de 0 à 100
 */
void SERVO_set_position(uint16_t position) {
	if (position > 100) position = 100;
	current_position = position;
	uint8_t duty = position + 100;
	BSP_TIMER_set_duty(TIMER1_ID, TIM_CHANNEL_3, duty);
}

/**
 * @brief  Renvoie la position actuelle du servo moteur
 *
 * @return position du servo moteur de 0 à 100
 */
uint16_t SERVO_get_position(void) {
	return current_position;
}
