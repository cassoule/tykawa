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
#include "tft_ili9341/stm32g4_ili9341.h"
#include "tft_ili9341/stm32g4_xpt2046.h"
#include "NFC03A1/stm32g4_nfc03a1.h"

#include "screen.h"
#include "servo.h"
#include "nfc.h"

#include <stdio.h>

static volatile bool coffee_done = false;
static volatile bool coffee_open = false;
static volatile bool water_open = false;

ISO14443A_CARD infos;

void state_machine(void);
void test_NFC_State_Machine(void);
int u_choice_handler(void);
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
 * @brief Machine à état pour tester le fonctionnement du lecteur NFC
 */
void test_NFC_State_Machine(void){
	printf("enterring NFC state machine\n");

	typedef enum {
		INIT = 0,
		WAITING_NFC,
		STATE_SEND_NFC,
		END_RECEPTION,
		ERROR_NFC
	} state_e;



	static state_e state = INIT;
	uint16_t taille;
	char contenu[256];

	switch (state){
		case INIT :
			printf("init NFC\n");
			state = WAITING_NFC;
			break;

		case WAITING_NFC :
			if(receive_NFC_Data(&infos)){
				printf("Frame received\n");
				state=STATE_SEND_NFC;
			}
			break;

		case STATE_SEND_NFC:{
			for(int i=0; i<infos.UIDsize; i++){
				contenu[i] = infos.UID[i];
			}
			taille = infos.UIDsize;
			state = END_RECEPTION;
			break;

		case END_RECEPTION :

			state = WAITING_NFC;
			break;

		case ERROR_NFC :
			printf("Error : No data Received while waiting data from NFC");
			state = ERROR_NFC;
			break;

	}
}}


/**
 * @brief  Machine à état de la machine à café Tykawa
 */

void state_machine(void) {
	typedef enum {
		INIT = 0,
		IDLE_CHOICE,
		PAYMENT,
		WORKING,
		DONE
	} state_e;

	const uint32_t COFFEE_DURATION_US = 2000000;

	static state_e state = INIT;
	static uint8_t u_choice = 0;
	static uint32_t delay = 0;
	uint16_t taille;
	char contenu[256];

	switch (state) {
		case INIT:
			printf("enter in init\n");

			SCREEN_init();
			SCREEN_draw_idle_screen();

			state = IDLE_CHOICE;
			break;

		case IDLE_CHOICE: {
			int choice = u_choice_handler();
			printf("enter in idle choice\n");
			if (choice) { //instead of : choice != 0
				u_choice = choice;
				delay = 0;
				printf("%d\n",choice);
				SCREEN_draw_payment_screen(u_choice);
				state = PAYMENT;
			}
			break;
		}

		case PAYMENT: {
			delay++;
			BSP_NFC03A1_Init(PCD);
			printf("enter in payment\n");
			if(receive_NFC_Data(&infos)){
				printf("NFC frame received");
				SCREEN_draw_working_screen(u_choice);
				uint32_t duration = COFFEE_DURATION_US * u_choice;
				coffee_done = false;
				BSP_TIMER_run_us(TIMER2_ID, duration, true);
				open_coffee_servo();
				// TODO: Ouverture vanne
				state = WORKING;

			}

			if (delay == 5000000) {
				delay = 0;
				SCREEN_draw_idle_screen();
				state = IDLE_CHOICE;
			}
			// TODO: Handle NFC
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
				delay = 0;
				SCREEN_draw_idle_screen();
				state = IDLE_CHOICE;
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
 * @brief Gestion du choix du café par l'utilisateur
 */
int u_choice_handler(){

	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) {
	    printf("btn left received\n");
	    return 1;

	    }
	else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET) {
	    printf("btn right received\n");
	    return 2;

	}
	printf("no btn received\n");
	return 0;
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
