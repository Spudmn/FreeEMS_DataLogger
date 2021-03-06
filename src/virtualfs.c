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


#include "diskio.h"		/* Common include file for FatFs and disk I/O layer */


/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>
//#include <device.h>				/* Include device specific declareation file here */

//#define	INIT_PORT()	{ init_port(); }	/* Initialize MMC control port (CS/CLK/DI:output, DO/WP/INS:input) */
//#define DLY_US(n)	{ dly_us(n); }		/* Delay n microseconds */
//
//#define	CS_H()		bset(P0)	/* Set MMC CS "high" */
//#define CS_L()		bclr(P0)	/* Set MMC CS "low" */
//#define CK_H()		bset(P1)	/* Set MMC SCLK "high" */
//#define	CK_L()		bclr(P1)	/* Set MMC SCLK "low" */
//#define DI_H()		bset(P2)	/* Set MMC DI "high" */
//#define DI_L()		bclr(P2)	/* Set MMC DI "low" */
//#define DO			btest(P3)	/* Get MMC DO value (high:true, low:false) */

#define	INS			(1)			/* Socket: Card is inserted (yes:true, no:false, default:true) */
#define	WP			(0)			/* Socket: Card is write protected (yes:true, no:false, default:false) */



/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command (SPI mode) */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD41	(41)		/* SEND_OP_COND (ACMD) */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */


static
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static
BYTE CardType;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */




/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void)	/* 1:OK, 0:Timeout */
{
//	BYTE d;
//	UINT tmr;
//
//
//	for (tmr = 5000; tmr; tmr--) {	/* Wait for ready in timeout of 500ms */
//		rcvr_mmc(&d, 1);
//		if (d == 0xFF) break;
//		DLY_US(100);
//	}
//
//	return tmr ? 1 : 0;

	return 1;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
//	BYTE d;
//
//	CS_H();
//	rcvr_mmc(&d, 1);	/* Dummy clock (force DO hi-z for multiple slave SPI) */
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

static
int SPI_select (void)	/* 1:OK, 0:Timeout */
{
//	BYTE d;
//
//	CS_L();
//	rcvr_mmc(&d, 1);	/* Dummy clock (force DO enabled) */
//
//	if (wait_ready()) return 1;	/* OK */
//	deselect();
	return 0;	/* Timeout */
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (	/* 1:OK, 0:Failed */
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count */
)
{
	BYTE d[2];
	UINT tmr;


	for (tmr = 1000; tmr; tmr--) {	/* Wait for data packet in timeout of 100ms */
		rcvr_mmc(d, 1);
		if (d[0] != 0xFF) break;
		//DLY_US(100);
	}
	if (d[0] != 0xFE) return 0;		/* If not valid data token, return with error */

	rcvr_mmc(buff, btr);			/* Receive the data block into buffer */
	rcvr_mmc(d, 2);					/* Discard CRC */

	return 1;						/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/

static
int xmit_datablock (	/* 1:OK, 0:Failed */
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE d[2];


	if (!wait_ready()) return 0;

	d[0] = token;
	xmit_mmc(d, 1);				/* Xmit a token */
	if (token != 0xFD) {		/* Is it data token? */
		xmit_mmc(buff, 512);	/* Xmit the 512 byte data block to MMC */
		rcvr_mmc(d, 2);			/* Xmit dummy CRC (0xFF,0xFF) */
		rcvr_mmc(d, 1);			/* Receive data response */
		if ((d[0] & 0x1F) != 0x05)	/* If not accepted, return with error */
			return 0;
	}

	return 1;
}



/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (		/* Returns command response (bit7==1:Send failed)*/
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, d, buf[6];


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		n = send_cmd(CMD55, 0);
		if (n > 1) return n;
	}

	/* Select the card and wait for ready */
	deselect();
	if (!SPI_select()) return 0xFF;

	/* Send a command packet */
	buf[0] = 0x40 | cmd;			/* Start + Command index */
	buf[1] = (BYTE)(arg >> 24);		/* Argument[31..24] */
	buf[2] = (BYTE)(arg >> 16);		/* Argument[23..16] */
	buf[3] = (BYTE)(arg >> 8);		/* Argument[15..8] */
	buf[4] = (BYTE)arg;				/* Argument[7..0] */
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;		/* (valid CRC for CMD0(0)) */
	if (cmd == CMD8) n = 0x87;		/* (valid CRC for CMD8(0x1AA)) */
	buf[5] = n;
	xmit_mmc(buf, 6);

	/* Receive command response */
	if (cmd == CMD12) rcvr_mmc(&d, 1);	/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		rcvr_mmc(&d, 1);
	while ((d & 0x80) && --n);

	return d;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv			/* Drive number (always 0) */
)
{
	DSTATUS s = Stat;
//	BYTE ocr[4];


//	if (drv || !INS) {
//		s = STA_NODISK | STA_NOINIT;
//	} else {
//		s &= ~STA_NODISK;
//		if (WP)						/* Check card write protection */
//			s |= STA_PROTECT;
//		else
//			s &= ~STA_PROTECT;
//		if (!(s & STA_NOINIT)) {
//			if (send_cmd(CMD58, 0))	/* Check if the card is kept initialized */
//				s |= STA_NOINIT;
//			rcvr_mmc(ocr, 4);
//			CS_H();
//		}
//	}
//	Stat = s;

	return s;
}



/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	DSTATUS s;

	s &= ~STA_NOINIT;
	Stat = s;
	return s;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..128) */
)
{
	DSTATUS s;


	s = disk_status(drv);
	if (s & STA_NOINIT) return RES_NOTRDY;
	if (!count) return RES_PARERR;


	FILE *fp;

		sector = (sector * 512) ;

		if( ( fp = fopen( "./Disk_Image/virtualfs", "r")) == NULL) {
		       puts("Cannot open the file");
		       return ( 0 );
		   }

		fseek (fp,sector,SEEK_SET);
		 fread(buff, 512, count, fp);
		 count = 0;

		fclose(fp);

//
//	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert LBA to byte address if needed */
//
//	if (count == 1) {	/* Single block read */
//		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
//			&& rcvr_datablock(buff, 512))
//			count = 0;
//	}
//	else {				/* Multiple block read */
//		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
//			do {
//				if (!rcvr_datablock(buff, 512)) break;
//				buff += 512;
//			} while (--count);
//			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
//		}
//	}
//	deselect();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..128) */
)
{
	DSTATUS s;


	s = disk_status(drv);
	if (s & STA_NOINIT) return RES_NOTRDY;
	if (s & STA_PROTECT) return RES_WRPRT;
	if (!count) return RES_PARERR;

//	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert LBA to byte address if needed */
//
//
//	if (count == 1) {	/* Single block write */
//		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
//			&& xmit_datablock(buff, 0xFE))
//			count = 0;
//	}
//	else {				/* Multiple block write */
//		if (CardType & CT_SDC) send_cmd(ACMD23, count);
//		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
//			do {
//				if (!xmit_datablock(buff, 0xFC)) break;
//				buff += 512;
//			} while (--count);
//			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
//				count = 1;
//		}
//	}
//	deselect();

	FILE *fp;

		sector = (sector * 512) ;

		if( ( fp = fopen( "./Disk_Image/virtualfs","rb+")) == NULL) {
		       puts("Cannot open the file");
		       return ( 0 );
		   }

		fseek (fp,sector,SEEK_SET);
		fwrite(buff, 512, count, fp);
		 count = 0;

		fclose(fp);

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16];
	WORD cs;


	if (disk_status(drv) & STA_NOINIT)					/* Check if card is in the socket */
		return RES_NOTRDY;

	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
				res = RES_PARERR;
				//res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			*(DWORD*)buff = 128;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
	}

	deselect();

	return res;
}

