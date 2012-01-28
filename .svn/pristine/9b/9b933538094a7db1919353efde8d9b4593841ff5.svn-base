/*------------------------------------------------------------------------/
/  Sound Streamer and RIFF-WAVE file player for LPC176x
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include <string.h>
#include "LPC1700.h"
#include "sound.h"
#include "xprintf.h"
#include "uart.h"


#define F_CCLK		100000000	/* CCLK frequency */
#define F_PCLK_PWM1	100000000	/* PCLK frequency to be supplied to PWM1 module */
#define F_PCLK_T3	25000000	/* PCLK frequency to be supplied to TIMER1 module */

#define NBSIZE 64
#define FCC(c1,c2,c3,c4)	((c4<<24)+(c3<<16)+(c2<<8)+c1)	/* FourCC */

#if F_PCLK_PWM1 * 1 == F_CCLK
#define PCLKDIV_PWM1	PCLKDIV_1
#elif F_PCLK_PWM1 * 2 == F_CCLK
#define PCLKDIV_PWM1	PCLKDIV_2
#elif F_PCLK_PWM1 * 4 == F_CCLK
#define PCLKDIV_PWM1	PCLKDIV_4
#elif F_PCLK_PWM1 * 8 == F_CCLK
#define PCLKDIV_PWM1	PCLKDIV_8
#else
#error Invalid frequency combination
#endif

#if F_PCLK_T3 * 1 == F_CCLK
#define PCLKDIV_T3	PCLKDIV_1
#elif F_PCLK_T3 * 2 == F_CCLK
#define PCLKDIV_T3	PCLKDIV_2
#elif F_PCLK_T3 * 4 == F_CCLK
#define PCLKDIV_T3	PCLKDIV_4
#elif F_PCLK_T3 * 8 == F_CCLK
#define PCLKDIV_T3	PCLKDIV_8
#else
#error Invalid frequency combination
#endif


static
WAVFIFO *WavFifo;	/* Pointer to sound FIFO control block */


/*-----------------------------------------------------*/
/* Sound sampling ISR                                  */

void TIMER3_IRQHandler (void)
{
	WAVFIFO *fcb = WavFifo;	/* Pointer to FIFO controls */
	UINT ri, ct;
	BYTE *buff;
	DWORD a1, a2;


	T3IR = T3IR;	/* Clear MR0 match irq flag */

	if (!fcb) return;
	buff = fcb->buff;
	ct = fcb->ct; ri = fcb->ri;

	switch (fcb->mode) {
	case 0:		/* Mono, 8bit */
		if (ct < 1) return;
		a1 = a2 = buff[ri] << 8;
		ct -= 1; ri += 1;
		break;
	case 1:		/* Stereo, 8bit */
		if (ct < 2) return;
		a1 = buff[ri] << 8;
		a2 = buff[ri + 1] << 8;
		ct -= 2; ri += 2;
		break;
	case 2:		/* Mono, 16bit */
		if (ct < 2) return;
		a1 = a2 = LD_WORD(buff+ri) ^ 0x8000;
		ct -= 2; ri += 2;
		break;
	default:	/* Stereo, 16bit */
		if (ct < 4) return;
		a1 = LD_WORD(buff+ri) ^ 0x8000;;
		a2 = LD_WORD(buff+ri+2) ^ 0x8000;;
		ct -= 4; ri += 4;
	}
	fcb->ct = ct;
	fcb->ri = ri & (fcb->sz_buff - 1);

	/* a1:L-ch, a2:R-ch */
	a1 >>= 7; a2 >>= 7;
	if (a1 == 511) a1 = 510;
	if (a2 == 511) a2 = 510;
	PWM1MR1 = a1 >> 1;
	PWM1MR2 = (~(a1 >> 1) - (a1 & 1)) & 0xFF;
	PWM1MR3 = a2 >> 1;
	PWM1MR4 = (~(a2 >> 1) - (a2 & 1)) & 0xFF;
	PWM1LER = 0x1F;
}



/*-----------------------------------------------------*/
/* Enable sound output stream                          */

int sound_start (
	WAVFIFO* fcb,	/* Pointer to the sound FIFO control structure */
	DWORD fs		/* Sampling frequency [Hz] */
)
{
	if (fs < 8000 || fs > 48000) return 0;	/* Check fs range */

	fcb->ri = 0; fcb->wi = 0; fcb->ct = 0;	/* Flush FIFO */
	WavFifo = fcb;			/* Register FIFO control structure */

	/* Configure PWM1 as audio outputs */
	__set_PCONP(PCPWM1, 1);		/* Enable PWM1 module */
	__set_PCLKSEL(PCLK_PWM1, PCLKDIV_PWM1);
	PWM1TCR = _BV(1);
	PWM1MCR = _BV(1);
	PWM1MR0 = 256 - 1;
	PWM1MR1 = 128;
	PWM1MR2 = 128;
	PWM1MR3 = 128;
	PWM1MR4 = 128;
	PWM1PCR = _BV(12)|_BV(11)|_BV(10)|_BV(9);
	PWM1TCR = _BV(3)|_BV(0);
	__set_PINSEL(2, 0, 1);
	__set_PINSEL(2, 1, 1);
	__set_PINSEL(2, 2, 1);
	__set_PINSEL(2, 3, 1);
	FIO2DIR |= _BV(3)|_BV(2)|_BV(1)|_BV(0);

	/* Configure TIMER3 as sampling interval timer */
	__set_PCONP(PCTIM3, 1);		/* Enable TIMER3 module */
	__set_PCLKSEL(PCLK_TIMER3, PCLKDIV_T3);
	T3TCR = _BV(1);
	T3MR0 = F_PCLK_T3 / fs - 1;
	T3MCR = _BV(1)|_BV(0);
	T3TCR = _BV(0);
	__enable_irqn(TIMER3_IRQn);	/* Enable interrupt */
	__set_irqn_priority(TIMER3_IRQn, PRI_HIGHEST);	/* Set interrupt priority, highest */

	return 1;
}



/*-----------------------------------------------------*/
/* Disable sound output                                */

void sound_stop (void)
{
	PWM1TCR = 0;				/* Stop PWM1 module */
	__set_PCONP(PCPWM1, 0);		/* Disable PWM1 module */
	__set_PINSEL(2, 0, 0);
	__set_PINSEL(2, 1, 0);
	__set_PINSEL(2, 2, 0);
	__set_PINSEL(2, 3, 0);
	FIO2CLR = _BV(3)|_BV(2)|_BV(1)|_BV(0);

	T3TCR = 0;					/* Stop sampling interrupt (TIMER3) */
	__disable_irqn(TIMER3_IRQn);
	__set_PCONP(PCTIM3, 0);		/* Disable TIMER3 module */

	WavFifo = 0;		/* Unregister FIFO control structure */
}



/*-----------------------------------------------------*/
/* WAV file loader                                     */

int load_wav (
	FIL *fp,			/* Pointer to the open file object to play */
	const char *title,	/* Title (file name, etc...) */
	void *work,			/* Pointer to working buffer (must be-4 byte aligned) */
	UINT sz_work		/* Size of working buffer (must be power of 2) */
)
{
	UINT md, wi, br, tc, t, btr;
	DWORD sz, ssz, offw, szwav, wsmp, fsmp, eof;
	WAVFIFO fcb;
	BYTE k, *buff = work;
	char *p, nam[NBSIZE], art[NBSIZE];


	xprintf("\f%s\n", title);	/* Put title */

	/* Is it a WAV file? */
	if (f_read(fp, buff, 12, &br) || br != 12) return -1;
	if (LD_DWORD(&buff[0]) != FCC('R','I','F','F')) return -1;
	if (LD_DWORD(&buff[8]) != FCC('W','A','V','E')) return -1;
	eof = LD_DWORD(&buff[4]) + 8;

	/* Analyze the RIFF-WAVE header and get properties */
	nam[0] = art[0] = 0;
	md = fsmp = wsmp = offw = szwav = 0;
	while (f_tell(fp) < eof) {
		if (f_read(fp, buff, 8, &br) || br != 8) return -1;
		sz = (LD_DWORD(&buff[4]) + 1) & ~1;
		switch (LD_DWORD(&buff[0])) {
		case FCC('f','m','t',' ') :
			if (sz > 1000 || sz < 16 || f_read(fp, buff, sz, &br) || sz != br) return -1;
			if (LD_WORD(&buff[0]) != 0x1) return -1;	/* Check if LPCM */
			if (LD_WORD(&buff[2]) == 2) {	/* Channels (1 or 2) */
				md = 1; wsmp = 2;
			} else {
				md = 0; wsmp = 1;
			}
			if (LD_WORD(&buff[14]) == 16) {	/* Resolution (8 or 16) */
				md |= 2; wsmp *= 2;
			}
			fsmp = LD_DWORD(&buff[4]);		/* Sampling rate */
			break;

		case FCC('f','a','c','t') :
			f_lseek(fp, f_tell(fp) + sz);
			break;

		case FCC('d','a','t','a') :
			offw = f_tell(fp);	/* Wave data start offset */
			szwav = sz;			/* Wave data length [byte] */
			f_lseek(fp, f_tell(fp) + sz);
			break;

		case FCC('L','I','S','T'):
			sz += f_tell(fp);
			if (f_read(fp, buff, 4, &br) || br != 4) return -1;
			if (LD_DWORD(buff) == FCC('I','N','F','O')) {	/* LIST/INFO chunk */
				while (f_tell(fp) < sz) {
					if (f_read(fp, buff, 8, &br) || br != 8) return -1;
					ssz = (LD_DWORD(&buff[4]) + 1) & ~1;
					p = 0;
					switch (LD_DWORD(buff)) {
					case FCC('I','N','A','M'):		/* INAM sub-chunk */
						p = nam; break;
					case FCC('I','A','R','T'):		/* IART sub-cnunk */
						p = art; break;
					}
					if (p && ssz <= NBSIZE) {
						if (f_read(fp, p, ssz, &br) || br != ssz) return -1;
					} else {
						if (f_lseek(fp, f_tell(fp) + ssz)) return -1;
					}
				}
			} else {
				if (f_lseek(fp, sz)) return -1;	/* Skip unknown sub-chunk type */
			}
			break;

		default :	/* Unknown chunk */
			return -1;
		}
	}
	if (!szwav || !fsmp) return -1;		/* Check if valid WAV file */
	if (f_lseek(fp, offw)) return -1;	/* Seek to top of wav data */

	xprintf("ART:%s\nNAM:%s\n", art, nam);	/* Put IART and INAM field */
	tc = szwav / fsmp / wsmp;
	xprintf("Len:%u:%02u, %u.%ukHz, %s\n", tc / 60, tc % 60, fsmp / 1000, (fsmp / 100) % 10, (md & 1) ? "½ÃÚµ" : "ÓÉ×Ù");

	/* Initialize stream parameters and start sound streming */
	fcb.mode = md;
	fcb.buff = buff;
	fcb.sz_buff = sz_work;
	if (!sound_start(&fcb, fsmp)) return -1;

	k = 0; wi = 0;
	while (szwav || fcb.ct >= 4) {
		if (szwav && fcb.ct <= sz_work / 2) {	/* Refill FIFO when it gets half empty */
			btr = (szwav >= sz_work / 2) ? sz_work / 2 : szwav;
			f_read(fp, &buff[wi], btr, &br);
			if (br != btr) break;
			szwav -= br;
			wi = (wi + br) & (sz_work - 1);
			__disable_irq();
			fcb.ct += br;
			__enable_irq();
		}
		if (uart_test()) {		/* Exit if a command arrived */
			k = uart_getc();
			break;
		}
		t = (f_tell(fp) - offw - fcb.ct) / fsmp / wsmp;	/* Refresh time display every 1 sec */
		if (t != tc) {
			tc = t;
			xprintf("%2u:%02u\r", tc / 60, tc % 60);
		}
	}

	sound_stop();	/* Stop sound output */

	xputc('\n');
	return k;	/* Terminated due to -1:error, 0:eot, >0:key code */
}


