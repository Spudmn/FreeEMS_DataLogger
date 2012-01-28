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
#ifndef SPI_H_
#define SPI_H_


static volatile
BYTE Timer1, Timer2;	/* 100Hz decrement timer */

BYTE rcvr_spi (void);



/* Alternative macro to receive data fast */
//#define rcvr_spi_m(dst)	SPDR=0xFF; loop_until_bit_is_set(SPSR,SPIF); *(dst)=SPDR
#define rcvr_spi_m(dst)	*(dst) = rcvr_spi()


/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

//#define xmit_spi(dat) 	SPDR=(dat); loop_until_bit_is_set(SPSR,SPIF)

void xmit_spi(BYTE dat);


#endif /* SPI_H_ */
