/*
 * screen.c
 *
 *  Created on: 27 avr. 2026
 *      Author: milog
 */
#include "config.h"
#include "tft_ili9341/stm32g4_ili9341.h"
#include "tft_ili9341/stm32g4_xpt2046.h"
#include "screen.h"

/**
 * @brief Initialise l'affichage de l'écran TFT dans un état neutre (écran blanc orienté correctement) ainsi que le tactile
 */
void SCREEN_init(void) {
	/* Affichage */
	ILI9341_Init();
	ILI9341_Rotate(ILI9341_Orientation_Landscape_2);
	SCREEN_clear();

	/* Tactile */
	ILI9341_setConfig();
	XPT2046_init();
}

/**
 * @brief Reset l'affichage de l'écran
 *
 */
void SCREEN_clear(void) {
	ILI9341_DisplayOff();
	ILI9341_DisplayOn();
	ILI9341_Fill(ILI9341_COLOR_WHITE);
}

/**
 * @brief Dessine l'écran de base, proposant un choix à l'utilisateur
 */
void SCREEN_draw_idle_screen(void) {
	SCREEN_clear();
	ILI9341_printf(15, 110, &Font_16x26, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Espresso");
	ILI9341_printf(190, 110, &Font_16x26, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Coffee");
	ILI9341_DrawFilledRectangle(165, 20, 167, 220, ILI9341_COLOR_BLACK);
}

/**
 * @brief Dessine l'écran de préparation avec une barre de progression vide
 * @param choice: choix initial de l'utilisateur à afficher (1 ou 2)
 */
void SCREEN_draw_working_screen(uint8_t choice) {
	SCREEN_clear();
	ILI9341_printf(15, 110, &Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Preparing: %s", (choice == 1 ? "Espresso" : "Coffee"));
	ILI9341_DrawFilledRectangle(15, 140, 290, 160, ILI9341_COLOR_BLACK);
}

/**
 * @brief Dessine la barre de progression de la préparation à un instant t
 * @param progress: état d'avancement (float de 0 à 1)
 */
void SCREEN_draw_progress_bar(float progress) {
	uint16_t length = 280;
	ILI9341_DrawFilledRectangle(15, 140, 15 + (progress * length), 160, ILI9341_COLOR_YELLOW);
}

/**
 * @brief Dessine l'écran de fin de préparation
 * @param choice: choix initial de l'utilisateur à afficher (1 ou 2)
 */
void SCREEN_draw_done_screen(uint8_t choice) {
	SCREEN_clear();
	ILI9341_printf(15, 110, &Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, "Your %s is ready ! :)", (choice == 1 ? "Espresso" : "Coffee"));
}

/**
 * @brief Cette fonction gère le clic d'un utilisateur sur l'écran tactile
 * @return Le choix de l'utilisateur (1 ou 2), 0 si l'utilisateur n'a fait aucun choix
 */
uint8_t SCREEN_handle_click(void) {
	int16_t x, y;
	if(XPT2046_getMedianCoordinates(&x, &y, XPT2046_COORDINATE_SCREEN_RELATIVE))
	{
		return y < 120 ? 1 : 2;
	}
	return 0;
}
