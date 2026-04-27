/**
 *******************************************************************************
 * @file 	main.c
 * @author 	jjo
 * @date 	Mar 29, 2024
 * @brief	Fichier principal de votre projet sur carte Nucléo STM32G431KB
 *******************************************************************************
 */

#include "config.h"
#include "stm32g4_sys.h"

#include "stm32g4_timer.h"
#include "stm32g4_systick.h"
#include "stm32g4_gpio.h"
#include "stm32g4_uart.h"
#include "stm32g4_utils.h"

#include "screen.h"
#include "servo.h"

#include <stdio.h>

static volatile bool coffee_done = false;
static volatile bool coffee_open = false;
static volatile bool water_open = false;

void state_machine(void);
void open_coffee_servo(void);
void close_coffee_servo(void);
void TIMER2_user_handler_it(void);

/**
  * @brief Point d'entrée de l'application
  */
int main(void)
{
	HAL_Init();
	BSP_GPIO_enable();
	BSP_UART_init(UART2_ID,115200);
	BSP_SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);
	SERVO_init();

	while (1)
	{
		state_machine();
	}
}

/**
 * @brief  Machine à état de la machine à café Tykawa
 */
void state_machine(void) {
	typedef enum {
		INIT = 0,
		IDLE,
		WORKING,
		DONE
	} state_e;

	const uint32_t COFFEE_DURATION_US = 2000000;

	static state_e state = INIT;
	static uint8_t u_choice = 0;
	static uint32_t delay = 0;

	switch (state) {
		case INIT:
			SCREEN_init();
			SCREEN_draw_idle_screen();
			state = IDLE;
			break;

		case IDLE: {
			int choice = SCREEN_handle_click();
			if (choice != 0) {
				u_choice = choice;
				SCREEN_draw_working_screen(u_choice);
				uint32_t duration = COFFEE_DURATION_US * u_choice;
				coffee_done = false;
				BSP_TIMER_run_us(TIMER2_ID, duration, true);
				open_coffee_servo();
				// TODO: Ouverture vanne
				state = WORKING;
			}
			break;
		}

		case WORKING: {
			uint32_t elapsed = BSP_TIMER_read(TIMER2_ID);
			uint32_t total = BSP_TIMER_get_period(TIMER2_ID);
			float progress = (float)elapsed / (float)total;

			SCREEN_draw_progress_bar(progress);

			uint32_t one_sec = total / (u_choice == 1 ? 2 : 4);
			if (elapsed > one_sec) close_coffee_servo();

			// TODO: fermeture vanne apres Y secondes
			if (coffee_done) {
				delay = 0;
				SCREEN_draw_done_screen(u_choice);
				state = DONE;
			}
			break;
		}

		case DONE:
			delay++;
			if (delay == 5000000) {
				SCREEN_draw_idle_screen();
				state = IDLE;
			}
			break;

		default:
			break;
	}
}

/**
 * @brief  Met le servo moteur à l'état ouvert s'il ne l'est pas déjà
 *
 */
void open_coffee_servo(void) {
	if (SERVO_get_position() != 100 && coffee_open == false) {
		SERVO_set_position(100);
		coffee_open = true;
	}
}


/**
 * @brief  Met le servo moteur à l'état fermé s'il ne l'est pas déjà
 *
 */
void close_coffee_servo(void) {
	if (SERVO_get_position() != 0 && coffee_open == true) {
		SERVO_set_position(0);
		coffee_open = false;
	}

}

/**
 * @brief  Cette fonction gère la fin du timer de préparation du café
 */
void TIMER2_user_handler_it(void) {
	coffee_done = true;
	BSP_TIMER_stop(TIMER2_ID);
}
