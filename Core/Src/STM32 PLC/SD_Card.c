/*
 * SD_Card.c
 *
 *  Created on: Jun 15, 2021
 *      Author: Daniel Mårtensson
 */
#include "Functions.h"
#include "user_diskio_spi.h"
#include "fatfs.h"

/* These are extern inside fatfs.h */
FATFS USERFatFS;
FIL USERFile;

void STM32_PLC_Start_SD(SPI_HandleTypeDef *hspi, GPIO_TypeDef *SD_CS_PORT, uint16_t SD_CS_PIN) {
	SD_init(hspi, SD_CS_PORT, SD_CS_PIN);
}

FRESULT STM32_PLC_SD_Mont_Card() {
	FRESULT status;
	uint8_t attempt = 0;
	while(attempt < 255) {
		MX_FATFS_Init();
		status = f_mount(&USERFatFS, "", 1);
		if(status == FR_OK){
			break;
		} else {
			STM32_PLC_SD_Unmount_Card();
			attempt++;
		}
	}
	return status;
}

FRESULT STM32_PLC_SD_Unmount_Card() {
	FRESULT status = f_mount(NULL, "", 0);
	MX_FATFS_DeInit();
	return status;
}

FRESULT STM32_PLC_SD_Open_Existing_File_With_Read(char filename[]) {
	return f_open(&USERFile, filename, FA_READ); /* Posix "r" */
}

/* TODO: byt namn på detta till STM32_PLC_SD_Create_File_With_Write */
FRESULT STM32_PLC_SD_Open_Existing_File_With_Write(char filename[]) {
	return f_open(&USERFile, filename, FA_CREATE_ALWAYS | FA_WRITE); /* Posix "w" */
}

/* TODO: byt namn på detta */
FRESULT STM32_PLC_SD_Create_New_File_With_Read_Write(char filename[], char initial_string[], bool overwrite) {
	FRESULT status;
	if(overwrite){
		/* Will always create a new file, so status will always be FR_OK */
		status = f_open(&USERFile, filename, FA_CREATE_ALWAYS | FA_READ | FA_WRITE); /* Posix "w+" */
	}else {
		/* If filename exist, then status will be FR_EXIST, else FR_OK if not exist */
		status = f_open(&USERFile, filename, FA_CREATE_NEW | FA_READ | FA_WRITE); /* Posix "w+x" */
	}
	/* If status is FR_OK, then write the initial string */
	if(status == FR_OK) {
		STM32_PLC_SD_Write_File(initial_string);
		STM32_PLC_SD_Close_File();
	}
	return status;
}

FRESULT STM32_PLC_SD_Close_File() {
	return f_close(&USERFile);
}

FRESULT STM32_PLC_SD_Check_Space(uint32_t *total_space, uint32_t *free_space) {
	FATFS *pfs;
	DWORD fre_clust;
	FRESULT status = f_getfree("", &fre_clust, &pfs);
	*total_space = (uint32_t) ((pfs->n_fatent - 2) * pfs->csize * 0.5);
	*free_space = (uint32_t) (fre_clust * pfs->csize * 0.5);
	return status;
}

/* Return text "error" or "eof" if it's end of file (eof) */
char* STM32_PLC_SD_Read_File(char text[], int len) {
	return f_gets(text, len, &USERFile);
}

/* Return -1 with End of File (EOF) */
int STM32_PLC_SD_Write_File(char text[]) {
	return f_puts(text, &USERFile);
}
