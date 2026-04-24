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

#include <stdio.h>

/**
 * @brief Initialise le tactil du TFT (XPT2046)
 */
void init_TFT_touch() {
	ILI9341_setConfig();
	XPT2046_init();
}

/**
 * @brief Initialise l'écran TFT dans un état neutre (écran blanc orienté correctement)
 */
void init_TFT_display() {
	ILI9341_Init();
	ILI9341_Rotate(ILI9341_Orientation_Landscape_2);
	ILI9341_DisplayOff();
	ILI9341_DisplayOn();
	ILI9341_Fill(ILI9341_COLOR_WHITE);
}

/**
 * @brief Dessine l'écran de base, proposant un choix à l'utilisateur
 */
void draw_idle_screen() {
	ILI9341_DisplayOff();
	ILI9341_DisplayOn();
	ILI9341_Fill(ILI9341_COLOR_WHITE);
	ILI9341_printf(15, 110, &Font_16x26, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Espresso");
	ILI9341_printf(190, 110, &Font_16x26, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Coffee");
	ILI9341_DrawFilledRectangle(165, 20, 167, 220, ILI9341_COLOR_BLACK);
}

/**
 * @brief Dessine l'écran de préparation avec une barre de progression vide
 * @param choice: choix initial de l'utilisateur à afficher (1 ou 2)
 */
void draw_working_screen(uint8_t choice) {
	ILI9341_DisplayOff();
	ILI9341_DisplayOn();
	ILI9341_Fill(ILI9341_COLOR_WHITE);
	ILI9341_printf(15, 110, &Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Preparing: %s", (choice == 1 ? "Espresso" : "Coffee"));
	ILI9341_DrawFilledRectangle(15, 140, 290, 160, ILI9341_COLOR_BLACK);
}

/**
 * @brief Dessine la barre de progression de la préparation à un instant t
 * @param runtime: temps depuis le début de la préparation
 * @param end_at: durée totale de la préparation
 */
void draw_progress_bar(uint64_t runtime, uint64_t end_at) {
	int length = 280;
	int progress = (runtime * length) / end_at;
	ILI9341_DrawFilledRectangle(15, 140, 15 + progress, 160, ILI9341_COLOR_YELLOW);
}

/**
 * @brief Dessine l'écran de fin de préparation
 * @param choice: choix initial de l'utilisateur à afficher (1 ou 2)
 */
void draw_done_screen(uint8_t choice) {
	ILI9341_DisplayOff();
	ILI9341_DisplayOn();
	ILI9341_Fill(ILI9341_COLOR_WHITE);
	ILI9341_printf(15, 110, &Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Your %s is ready ! :)", (choice == 1 ? "Espresso" : "Coffee"));
}

/**
 * @brief Cette fonction gère le clic d'un utilisateur sur l'écran tactile
 * @return Le choix de l'utilisateur (1 ou 2), 0 si l'utilisateur n'a fait aucun choix
 */
uint8_t handle_click() {
	int16_t x, y;
	if(XPT2046_getMedianCoordinates(&x, &y, XPT2046_COORDINATE_SCREEN_RELATIVE))
	{
		return y < 120 ? 1 : 2;
	}
	return 0;
}

/**
 * @brief Cette fonction gère la fin du timer de préparation du café.
 */
void TIMER2_user_handler_it() {
	coffee_done = true;
	BSP_TIMER_stop(TIMER2_ID);
}

static volatile bool coffee_done = false;

/**
 * @brief Machine à état de la machine à café Tykawa
 */
void state_machine() {
	typedef enum {
		INIT = 0,
		IDLE,
		WORKING,
		DONE
	} state_e;

	static state_e state = INIT;
	static uint8_t u_choice = 0;
	static uint32_t COFFEE_DURATION_US = 2000000;
	static uint32_t delay = 0;

	switch (state) {
		case INIT:
			init_TFT_display();
			init_TFT_touch();
			draw_idle_screen();
			state = IDLE;
			break;

		case IDLE: {
			int choice = handle_click();
			if (choice != 0) {
				u_choice = choice;
				draw_working_screen(u_choice);

				uint32_t duration = COFFEE_DURATION_US * u_choice;
				coffee_done = false;
				BSP_TIMER_run_us(TIMER2_ID, duration, true);

				// TODO: Ouverture vanne et servo

				state = WORKING;
			}
			break;
		}

		case WORKING: {
			uint32_t elapsed = BSP_TIMER_read(TIMER2_ID);
			uint32_t total = BSP_TIMER_get_period(TIMER2_ID);
			// TODO: fermeture servo apres X secondes
			// TODO: fermeture vanne apres Y secondes
			draw_progress_bar(elapsed, total);
			if (coffee_done) {
				delay = 0;
				draw_done_screen(u_choice);
				state = DONE;
			}
			break;
		}

		case DONE:
			delay++;
			if (delay == 5000000) {
				draw_idle_screen();
				state = IDLE;
			}
			break;

		default:
			break;
	}
}

/**
  * @brief  Point d'entrée de votre application
  */
int main(void)
{
	/* Cette ligne doit rester la première de votre main !
	 * Elle permet d'initialiser toutes les couches basses des drivers (Hardware Abstraction Layer),
	 * condition préalable indispensable à l'exécution des lignes suivantes.
	 */
	HAL_Init();

	/* Initialisation des périphériques utilisés dans votre programme */
	BSP_GPIO_enable();
	BSP_UART_init(UART2_ID,115200);

	/* Indique que les printf sont dirigés vers l'UART2 */
	BSP_SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

	/* Initialisation du port de la led Verte (carte Nucleo) */
	BSP_GPIO_pin_config(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH,GPIO_NO_AF);

	/* Tâche de fond, boucle infinie, Infinite loop,... quelque soit son nom vous n'en sortirez jamais */
	while (1)
	{
		state_machine();
	}
}
