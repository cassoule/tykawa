/*
 * servo.h
 *
 *  Created on: 27 avr. 2026
 *      Author: milog
 */
#include "config.h"

#ifndef SERVO_H_
#define SERVO_H_

void SERVO_init(void);

void SERVO_set_position(uint16_t position);

uint16_t SERVO_get_position(void);

#endif /* SERVO_H_ */
