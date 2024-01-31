/*
 * splash.c
 *
 *  Created on: Dec 31, 2023
 *      Author: Joshua Butler, MD, MHI
 */

#include <stdio.h>
#include <string.h>
#include "snake_bitmap.h"
#include "splash.h"
#include "cmsis_os.h"

void splash(void) {

	uint8_t buffer[12];

	ssd1306_Init();
	ssd1306_SetContrast(20);
	ssd1306_Fill(Black);
	ssd1306_SetCursor(16, 1);
	ssd1306_WriteString("BUTLER", Font_16x26, White);
	ssd1306_SetCursor(3, 28);
	ssd1306_WriteString("ELECTRONICS", Font_11x18, White);
	ssd1306_SetCursor(0, 53);
	ssd1306_WriteString("(C)2024", Font_7x10, White);
	snprintf((char*) buffer, 12, "v%d.%d", VERSION_MAJOR, VERSION_MINOR);
	uint8_t len = strlen((char*) buffer);
	ssd1306_SetCursor(128 - (len * 7), 53);
	ssd1306_WriteString((char*) buffer, Font_7x10, White);
	ssd1306_UpdateScreen();
	osDelay(2000);

	ssd1306_Fill(Black);
	for(int i = 0; i < snake_bitmap_allArray_LEN; i++) {
		ssd1306_Fill(Black);
		ssd1306_DrawBitmap(0, 0, snake_bitmap_allArray[i], 128, 64, White);
		ssd1306_UpdateScreen();
		osDelay(150);
	}

	ssd1306_DrawBitmap(0, 0, snake_game_logo, 128, 64, White);
	ssd1306_SetCursor(25, 49);
	ssd1306_WriteString("Press Start", Font_7x10, White);
	ssd1306_UpdateScreen();

}

void draw_home_screen(void) {

    ssd1306_Init();
    ssd1306_UpdateScreen();
//    ssd1306_SetContrast(20);
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, snake_game_logo, 128, 64, White);
    ssd1306_SetCursor(25, 49);
    ssd1306_WriteString("Press Start", Font_7x10, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_UpdateScreen();
}
