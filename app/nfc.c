/*
 * nfc.c
 *
 *  Created on: 29 mai 2026
 *      Author: lucas
 */
#include "NFC03A1/stm32g4_nfc03a1.h"
#include "nfc.h"

/**
 * @brief fonction qui permet de renvoyer true lorsqu'une trame est reçue sur le NFC
 */
bool receive_NFC_Data(ISO14443A_CARD *infos){
	uint8_t tag;

	tag = ConfigManager_TagHunting(TRACK_ALL);
	switch(tag)
	{
		case TRACK_NFCTYPE4A:{
			BSP_NFC03A1_get_ISO14443A_infos(infos);
			printf("uid = ");
			uint8_t i;
			for(i=0; i<infos->UIDsize;i++)
				printf("%02x ",infos->UID[i]);
			printf("\n");
			return true;
			break;}
		default:
			return false;
			break;
	}
}
