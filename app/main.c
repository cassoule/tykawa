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

#include "stm32g4_systick.h"
#include "stm32g4_gpio.h"
#include "stm32g4_uart.h"
#include "stm32g4_utils.h"
#include "tft_ili9341/stm32g4_ili9341.h"
#include "tft_ili9341/stm32g4_xpt2046.h"

#include <stdio.h>

void write_LED(bool b)
{
	HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, b);
}

void init_TFT_display() {
	ILI9341_Rotate(ILI9341_Orientation_Landscape_2);
	ILI9341_DrawFilledRectangle(0, 0, 160, 240, ILI9341_COLOR_BLUE);
	ILI9341_DrawFilledRectangle(160, 0, 320, 240, ILI9341_COLOR_BLUE2);
	ILI9341_DrawFilledRectangle(160, 0, 160, 240, ILI9341_COLOR_BLACK);
	ILI9341_printf(15, 110, &Font_16x26, ILI9341_COLOR_BLACK, ILI9341_COLOR_BLUE, "Espresso");
	ILI9341_printf(190, 110, &Font_16x26, ILI9341_COLOR_BLACK, ILI9341_COLOR_BLUE2, "Coffee");
}

void init_TFT_touch() {
	ILI9341_setConfig();
	XPT2046_init();

}

enum state_e {
	INIT = 0,
	IDLE,
	WORKING
};

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

	/* Initialisation de l'écran TFT */
	ILI9341_Init();
	init_TFT_display();

	/* Initialisation du port de la led Verte (carte Nucleo) */
	BSP_GPIO_pin_config(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH,GPIO_NO_AF);

	/* Tâche de fond, boucle infinie, Infinite loop,... quelque soit son nom vous n'en sortirez jamais */
	while (1)
	{
		static int16_t static_x,static_y;
		int16_t x, y;

		if(XPT2046_getMedianCoordinates(&x, &y, XPT2046_COORDINATE_SCREEN_RELATIVE))
		{
			static_x = x;
			static_y = y;
			if (y < 120) {
				printf("Espresso\n");
			} else if (y >= 120) {
				printf("Coffee\n");
			}
		}
	}
}
