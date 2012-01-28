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

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "integer.h"

#define FALSE 0x00
#define TRUE  !FALSE

#define loop_until_bit_is_set(a,b)

#define SBIT(name, addr, bit)  volatile unsigned char  name
#define SFR(name)        volatile unsigned char  name
#define SFR16(name)      volatile unsigned short name

SFR(PORTB);
SFR(DDRB);
SFR(PORTE);
SFR(SPCR);
SFR(SPSR);
SFR(SPDR);
SFR(SPIF);

SBIT(PINB,0,1);


#endif /* GLOBAL_H_ */
