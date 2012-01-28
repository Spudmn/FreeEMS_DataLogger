/* FreeEMS - the open source engine management system
 *
 * Copyright 2012 Aaron Keith
 *
 * This file is part of the FreeEMS project.
 *
 * FreeEMS software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeEMS software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with any FreeEMS software.  If not, see http://www.gnu.org/licenses/
 *
 * We ask that if you make any changes to this file you email them upstream to
 * us at admin(at)diyefi(dot)org or, even better, fork the code on github.com!
 *
 * Thank you for choosing FreeEMS to run your engine!
 */

#include "../Global.h"
#include <stdio.h>

static volatile BYTE Timer1, Timer2; /* 100Hz decrement timer */

static DWORD Read_Sector;
BYTE Read_Sector_Buffer[512 + 2]; //Plus 2 for CRC not used
WORD Read_Sector_Index;

static DWORD Write_Sector;
BYTE Write_Sector_Buffer[512 + 2]; //Plus 2 for CRC not used
WORD Write_Sector_Index;
/*-----------------------------------------------------------------------*/
/* Transmit bytes to the card                               */
/*-----------------------------------------------------------------------*/

static
void xmit_mmc(const BYTE* buff, /* Data to be sent */
UINT bc /* Number of bytes to send */
) {

}

void vfs_disk_write(const BYTE *buff, /* Pointer to the data to be written */
DWORD sector, /* Start sector number (LBA) */
BYTE count /* Sector count (1..128) */
) {
	FILE *fp;

	//sector = (sector * 512);

	if ((fp = fopen("./Disk_Image/virtualfs", "rb+")) == NULL) {
		puts("Cannot open the file");
		return;
	}

	fseek(fp, sector, SEEK_SET);
	fwrite(buff, 512, count, fp);
	count = 0;

	fclose(fp);

}

void vfs_disk_read(BYTE *buff, /* Pointer to the data buffer to store read data */
DWORD sector, /* Start sector number (LBA) */
BYTE count /* Sector count (1..128) */
) {

	FILE *fp;

	//sector = (sector * 512) ;

	if ((fp = fopen("./Disk_Image/virtualfs", "r")) == NULL) {
		puts("Cannot open the file");
		return;
	}

	fseek(fp, sector, SEEK_SET);
	fread(buff, 512, count, fp);
	count = 0;

	fclose(fp);

}

//*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

typedef enum {
	CARD_START = 0,
	CARD_CMD_0,
	CARD_CMD_1,
	CARD_CMD_8,
	CARD_CMD_41,
	CARD_CMD_16,
	CARD_CMD_55,
	CARD_CMD_17,
	CARD_CMD_17_Token,
	CARD_CMD_17_Reading,
	CARD_CMD_24,
	CARD_CMD_24_RESULT

} Sim_State_t;

Sim_State_t Sim_State;

BYTE MMC_Simulator_RCVR(void) {
	BYTE Result;

	switch (Sim_State) {
	case CARD_START:
		Result = 0x00;
		break;

	case CARD_CMD_0:
		Result = 0x01;
		break;
	case CARD_CMD_1:
		Result = 0x00;
		break;
	case CARD_CMD_8:
		Result = 0x00; //send zero so that is mmc
		break;

	case CARD_CMD_41:
		Result = 0x2; //send some thing greater than 2 so that is mmc
		break;

	case CARD_CMD_16:
		Result = 0x00; //Set R/W block length to 512
		break;

	case CARD_CMD_55:
		Result = 0x02;
		break;

	case CARD_CMD_17:
		Result = 0x00; //SREAD_SINGLE_BLOCK
		Sim_State = CARD_CMD_17_Token;
		break;

	case CARD_CMD_17_Token:
		Result = 0xFE; //SREAD_SINGLE_BLOCK token
		Sim_State = CARD_CMD_17_Reading;
		break;

	case CARD_CMD_17_Reading:

		Result = Read_Sector_Buffer[Read_Sector_Index]; //read data from disk
		Read_Sector_Index++;
		break;

	case CARD_CMD_24:
		Result = 0x00; //WRITE_SINGLE_BLOCK
		Sim_State = CARD_CMD_24_RESULT;
		break;

	case CARD_CMD_24_RESULT:
//			Recived all the data. Flush to disk
		vfs_disk_write(Write_Sector_Buffer, Write_Sector, 1);
		Result = 0x05; //WRITE_SINGLE_BLOCK
//			Sim_State = CARD_CMD_24_RESULT;
		break;

	default:
		printf("Invalid Sim State");
		break;
	}

	return Result;

}

void xmit_spi(BYTE dat) {
	typedef enum {
		START_CMD = 0,
		CMD_ARG1,
		CMD_ARG2,
		CMD_ARG3,
		CMD_ARG4,
		CMD_CRC,
		WRITE_DATA
	} SPI_Xmit;

	static SPI_Xmit CMD_State = START_CMD;
	static BYTE CMD;
	static DWORD ARG;

	if (Timer1)
		Timer1--;
	if (Timer2)
		Timer2--;

	switch (CMD_State) {
	case START_CMD:
		if ((dat & 0x40) == 0x40) {
			CMD = dat & ~0x40;
			CMD_State = CMD_ARG1;
		}

		if (dat == 0xFE) { // data token
			CMD_State = WRITE_DATA;

		}
		break;

	case CMD_ARG1:
		ARG = (DWORD) (dat << 24);
		CMD_State = CMD_ARG2;
		break;

	case CMD_ARG2:
		ARG |= (DWORD) (dat << 16);
		CMD_State = CMD_ARG3;
		break;

	case CMD_ARG3:
		ARG |= (DWORD) (dat << 8);
		CMD_State = CMD_ARG4;
		break;

	case CMD_ARG4:
		ARG |= (DWORD) (dat << 16);
		CMD_State = CMD_CRC;
		if (CMD == 0x00) {
			Sim_State = CARD_CMD_0;
		} else if (CMD == 01) {
			Sim_State = CARD_CMD_1;
		} else if (CMD == 0x08) {
			Sim_State = CARD_CMD_8;
		}

		else if (CMD == 41) {
			Sim_State = CARD_CMD_41;
		} else if (CMD == 55) {
			Sim_State = CARD_CMD_55;
		} else if (CMD == 16) {
			Sim_State = CARD_CMD_16;
		} else if (CMD == 17) //read single sector
				{
			Sim_State = CARD_CMD_17;
			Read_Sector = ARG;
			vfs_disk_read(Read_Sector_Buffer, Read_Sector, 1);
			Read_Sector_Index = 0x00;

		} else if (CMD == 24) //write single sector
				{
			Sim_State = CARD_CMD_24;
			Write_Sector = ARG;
			//vfs_disk_read(Read_Sector_Buffer,Read_Sector,1);
			Write_Sector_Index = 0x00;

		} else {
			printf("Unknown Command %d", CMD);
		}
		break;

	case CMD_CRC:
//					ARG |= (DWORD)(dat << 16);
		CMD_State = START_CMD;

		break;

	case WRITE_DATA:
		Write_Sector_Buffer[Write_Sector_Index] = dat; //read data from disk
		Write_Sector_Index++;
		if (Write_Sector_Index >= 514) {
			CMD_State = START_CMD;
		}
		break;

	default:
		printf("Invalid Xmit State");
		break;
	}

}

BYTE rcvr_spi(void) {
//	SPDR = 0xFF;
//	loop_until_bit_is_set(SPSR, SPIF);
//	return SPDR;

	return MMC_Simulator_RCVR();

}

