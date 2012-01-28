/*--------------------------------------------------------------------------*/
/* NAND-FTL driver (via GPIO)                                 (C)ChaN, 2010 */
/*--------------------------------------------------------------------------*/
/* This program is opened under license policy of following trems.
/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This program is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial use UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/---------------------------------------------------------------------------*/

#include "LPC2300.h"
#include "diskio.h"


/* These functions are defined in asmfunc.S */
void Copy_al2un (BYTE *dst, const DWORD *src, int count);	/* Copy aligned to unaligned. */
void Copy_un2al (DWORD *dst, const BYTE *src, int count);	/* Copy unaligned to aligned. */


/* NAND flash configurations (depends on the memory chip) */

#define	N_TOTAL_BLKS	8192	/* Total number of blocks on the memory chip (8MB:512, 16MB:1024, 32MB:2048, 64MB:4096, 128MB:8192) */
#define	N_SIZE_BLK		32		/* Block size (Number of 528 byte sectors per erase block) */
#define	N_RES_BLKS		1		/* Number of top blocks to be reserved for system */
#define	P_SPARE_BLKS	3		/* Percentage of spare blocks in total blocks */

#define	N_SPARE_BLKS	(N_TOTAL_BLKS * P_SPARE_BLKS / 100)			/* Number of spare blocks */
#define	N_USER_BLKS		(N_TOTAL_BLKS - N_RES_BLKS - N_SPARE_BLKS)	/* Number of user blocks */
#define MAX_LBA			(N_USER_BLKS * N_SIZE_BLK)					/* Number of user sectors */
#define	LONG_ADR		(N_TOTAL_BLKS * N_SIZE_BLK >= 131072)		/* Long address device */



/* Module work area */

static
WORD BlockMap[N_TOTAL_BLKS] __attribute__ ((section(".etherram")));	/* Physical block status table (2 * N_TOTAL_BLKS bytes) */
/* 0xFFFF:Blank, 0xFFFE:Bad, 0-0xFFFD:Live (logical block number) */

static
DWORD SectorBuff[N_SIZE_BLK][132];	/* Sector buffer (528 * N_SIZE_BLK bytes) */
/* Physical sector organization */
/*
/ DWORD : Description 
/ 0-63  : Sector data 1 (256 bytes)
/ 64-127: Sector data 2 (256 bytes)
/ 128   : b31-16: ~[b15-0]
/         b15-0:  Logical block number
/ 129   : b31-0: Bad Block Mark (!=0xFFFFFFFF:Bad)
/ 130   : b31-24: Not used (Don't care)
/         b21-0:  ECC of sector data 1
/ 131   : b31-24: Not used (Don't care)
/         b21-0:  ECC of sector data 2
*/


static
BYTE SectStat[N_SIZE_BLK];			/* Sector status in the block buffer (1:ECC checked) */

static
WORD LogBlock, PhyBlock, BadBlocks;	/* Current block status */
static
BYTE Init, BlockDirty;



/*--------------------------------------------------------------------------*/
/* NAND flash media access functions via GPIO (platform dependent)          */
/*--------------------------------------------------------------------------*/

#define NAND_DATA_OUT	FIO3PIN0			/* Set D0..7 direction output */
#define NAND_DATA_IN	FIO3PIN0			/* Set D0..7 direction input */
#define NAND_DIR_IN()	{FIO3DIR0=0;}		/* Get D0..7 input */
#define NAND_DIR_OUT()	{FIO3DIR0=0xFF;}	/* Set a byte to D0..7 */

#define NAND_READY		(FIO2PIN0 & 0x80)	/* Check ready signal (True:ready, False:busy) */

#define NAND_CTRL_OUT()	{FIO2DIR0 = 0x3F;}	/* Set CLE/ALE/RE/WE/CE/WP as output */
#define NAND_CLE_H()	{FIO2SET0=0x01;}
#define NAND_CLE_L()	{FIO2CLR0=0x01;}
#define NAND_ALE_H()	{FIO2SET0=0x02;}
#define NAND_ALE_L()	{FIO2CLR0=0x02;}
#define NAND_RE_H()		{FIO2SET0=0x04;}
#define NAND_RE_L()		{FIO2CLR0=0x04;}
#define NAND_WE_H()		{FIO2SET0=0x08;}
#define NAND_WE_L()		{FIO2CLR0=0x08;}
#define NAND_CE_H()		{FIO2SET0=0x10;}
#define NAND_CE_L()		{FIO2CLR0=0x10;}
#define NAND_WP_H()		{FIO2SET0=0x20;}
#define NAND_WP_L()		{FIO2CLR0=0x20;}


/*-------------------------------------*/
/* Wait until memory chip goes ready   */

static
void nand_wait (void)
{
	while (!NAND_READY) ;
}


/*-------------------------------------*/
/* Send a command byte to the memory   */

static
void nand_cmd (
	BYTE cmd
)
{
	NAND_DATA_OUT = cmd;
	NAND_DIR_OUT();
	NAND_CLE_H();
	NAND_WE_L(); NAND_WE_L();
	NAND_WE_H(); NAND_WE_H();
	NAND_CLE_L();
}


/*-------------------------------------*/
/* Send an address byte to the memory  */

static
void nand_adr (
	BYTE cmd
)
{
	NAND_DATA_OUT = cmd;
	NAND_DIR_OUT();
	NAND_ALE_H();
	NAND_WE_L(); NAND_WE_L();
	NAND_WE_H(); NAND_WE_H();
	NAND_ALE_L();
}


/*-------------------------------------*/
/* Read a byte from the memory         */

static
BYTE nand_read_byte (void)
{
	BYTE rb;


	NAND_DIR_IN();
	NAND_RE_L(); NAND_RE_L(); NAND_RE_L();
	rb = NAND_DATA_IN;
	NAND_RE_H();

	return rb;
}


/*-------------------------------------*/
/* Read multiple byte from the memory  */

static
void nand_read_multi (
	BYTE* buff,		/* Pointer to the buffer to store the read data */
	UINT bc			/* Number of bytes to read */
)
{
	NAND_DIR_IN();
	do {
		NAND_RE_L(); NAND_RE_L(); NAND_RE_L();
		*buff++ = NAND_DATA_IN;
		NAND_RE_H();
		NAND_RE_L(); NAND_RE_L(); NAND_RE_L();
		*buff++ = NAND_DATA_IN;
		NAND_RE_H();
	} while (bc -= 2);
}


/*-------------------------------------*/
/* Write multiple byte to the memory   */

static
void nand_write_multi (
	const BYTE* buff,	/* Pointer to the data to be written */
	UINT bc				/* Number of bytes to write */
)
{
	NAND_DIR_OUT();
	do {
		NAND_DATA_OUT = *buff++;
		NAND_WE_L(); NAND_WE_L();
		NAND_WE_H();
		NAND_DATA_OUT = *buff++;
		NAND_WE_L(); NAND_WE_L();
		NAND_WE_H();
	} while (bc -= 2);
}


/*-------------------------------------*/
/* Initialize memory I/F               */

static
void nand_init (void)
{
	NAND_CE_H();
	NAND_ALE_L();
	NAND_CLE_L();
	NAND_RE_H();
	NAND_WE_H();
	NAND_CTRL_OUT();
	NAND_CE_L();
	NAND_WP_H();

	nand_cmd(0xFF);
}



/*--------------------------------------------------------------------------*/
/* NAND flash control functions                                             */
/*--------------------------------------------------------------------------*/

/*-------------------------------------*/
/* Read sector(s)                      */

static
void read_sector (
	DWORD* buff,	/* Read data buffer */
	DWORD sa,		/* Physical sector address */
	UINT sc,		/* Number of sectors to read */
	UINT bs			/* Number of bytes per sector (4,8,12,16 or 528) */
)
{
	nand_wait();
	nand_cmd(bs <= 16 ? 0x50 : 0x00);	/* Read mode 1 or 3 */
	nand_adr(0);
	nand_adr((BYTE)(sa));
	nand_adr((BYTE)(sa >> 8));
	if (LONG_ADR) nand_adr((BYTE)(sa >> 16));
	do {
		nand_wait();
		nand_read_multi((BYTE*)buff, bs);
		buff += bs / 4;
	} while (--sc);
}



/*-------------------------------------*/
/* Write sector(s)                     */

static
int write_sector (		/* 0:Failed, 1:Successful */
	const DWORD* buff,	/* 528 byte sectors to be written */
	DWORD sa,			/* Physical sector address */
	UINT sc				/* Number of sectors to write */
)
{
	do {
		nand_wait();
		nand_cmd(0x80);				/* Data load command */
		nand_adr(0);				/* Physical sector address */
		nand_adr((BYTE)(sa));
		nand_adr((BYTE)(sa >> 8));
		if (LONG_ADR) nand_adr((BYTE)(sa >> 16));
		nand_write_multi((const BYTE*)buff, 528);	/* Load a sector data */
		nand_cmd(0x10);				/* Initiate to write */
		nand_wait();
		nand_cmd(0x70);				/* Check write result */
		if (nand_read_byte() & 0x01) return 0;
		buff += 132;				/* Next sector data */
		sa++;						/* Next sector address */
	} while (--sc);

	return 1;
}



/*-------------------------------------*/
/* Erase a physical block              */

static
int erase_phy_block (	/* 0:Failed, 1:Successful */
	WORD blk			/* Physical block to be erased */
)
{
	DWORD sa = (DWORD)blk * N_SIZE_BLK;

//	xprintf("[E:%d]", blk);
	nand_wait();
	nand_cmd(0x60);				/* Erase command */
	nand_adr((BYTE)(sa));		/* Physical block address */
	nand_adr((BYTE)(sa >> 8));
	if (LONG_ADR) nand_adr((BYTE)(sa >> 16));
	nand_cmd(0xD0);				/* Initiate to erase */
	nand_wait();
	nand_cmd(0x70);				/* Check erase result */

	BlockMap[blk] = 0xFFFF;		/* Mark the block 'blanked' */

	return (nand_read_byte() & 0x01) ? 0 : 1;
}



/*-----------------------------------------------------*/
/* Find and load a logical block into the block buffer */

static
void load_block (
	WORD lb,		/* Logical block number to be loaded */
	int load		/* 1:Load, 0:Do not load because the entire block is to be overwritten */
)
{
	UINT pb, i, j;


	/* Find the logical block */
	for (pb = N_RES_BLKS; pb < N_TOTAL_BLKS && BlockMap[pb] != lb; pb++) ;

	if (pb == N_TOTAL_BLKS) {	/* The logical block is not exist */
		for (i = 0; i < N_SIZE_BLK; i++) {	/* Initialize the block data with blank data */
			for (j = 0; j < 132; j++)
				SectorBuff[i][j] = 0xFFFFFFFF;
		}
		pb = 0;		/* No physical block */
	//	xprintf("[L:%d,B]", lb);
	} else {	/* Found the logical block */
		if (load)
			read_sector(SectorBuff[0], (DWORD)pb * N_SIZE_BLK, N_SIZE_BLK, 528);
	//	xprintf("[L:%d,L]", lb);
	}

	for (i = 0; i < N_SIZE_BLK; i++)	/* Initialize the logical block number in the spare area */
		SectorBuff[i][128] = (~lb << 16) | lb;

	PhyBlock = pb;	/* Current physical block */
	LogBlock = lb;	/* Current logical block */
	BlockDirty = 0;
	for (i = 0; i < N_SIZE_BLK; i++) SectStat[i] = 0;
}



/*------------------------------------*/
/* Mark an erase block as 'bad block' */

static
void bad_block (
	WORD pb			/* Physical block to be marked bad */
)
{
	DWORD sa;
	DWORD buf[4];
	int i;


//	xprintf("[B:%d]", pb);
	buf[1] = 0xFFFFFFFF;
	buf[2] = 0xFFFFFFFF;
	buf[3] = 0xFFFFFFFF;

	for (sa = (DWORD)pb * N_SIZE_BLK; sa / N_SIZE_BLK == pb; sa++) {
		nand_wait();
		nand_cmd(0x80);				/* Data load command */
		nand_adr(0);				/* Physical sector address */
		nand_adr((BYTE)(sa));
		nand_adr((BYTE)(sa >> 8));
		if (LONG_ADR) nand_adr((BYTE)(sa >> 16));
		buf[1] = 0xFFFFFFFF;		/* Load data area (not programmed) */
		for (i = 0; i < 32; i++)
			nand_write_multi((const BYTE*)buf, 16);
		buf[1] = 0;					/* Mark bad block mark */
		nand_write_multi((const BYTE*)buf, 16);
		nand_cmd(0x10);				/* Initiate to write */
	}
	nand_wait();

	BlockMap[pb] = 0xFFFE;			/* Mark the block 'bad' */
	BadBlocks++;					/* Increase bad block counter */
}



/*----------------------------------------------*/
/* Write-back block buffer into memory          */

static
int flush_block (void)
{
	WORD pb, pbs;


	if (BlockDirty) {
		pb = pbs = PhyBlock ? PhyBlock : N_RES_BLKS;	/* Find a free block */
		for (;;) {
			if (BadBlocks >= N_SPARE_BLKS) return 0;	/* Too many bad blocks */
			while (BlockMap[pb] != 0xFFFF) {			/* Find a free block */
				pb++;
				if (pb >= N_TOTAL_BLKS) pb = N_RES_BLKS;
				if (pb == pbs) return 0;				/* No free block is available */
			}
		//	xprintf("[W:%d]", pb);
			if (write_sector(SectorBuff[0], (DWORD)pb * N_SIZE_BLK, N_SIZE_BLK)) break;	/* Write-back block */
			bad_block(pb);		/* Mark 'bad-block' if the block goes bad */
		}
		BlockMap[pb] = LogBlock;

		if (PhyBlock) {	/* Erase old block */
			if (!erase_phy_block(PhyBlock)) bad_block(PhyBlock);
		}

		PhyBlock = pb;
		BlockDirty = 0;
	}

	return 1;
}



/*----------------------------------------------*/
/* Force blanked logical blocks                 */

static
DRESULT blank_log_blocks (
	DWORD *region		/* Block of logical sectors to be blanked {start, end} */
)						/* If start sector is not top of a logical block, the block is left not blanked. */
{						/* If end sector is not end of a logical block, the block is left not blanked. */
	WORD stlb, edlb, pb;


	if (region[1] >= MAX_LBA || region[0] > region[1]) return RES_PARERR;
	stlb = (WORD)((region[0] + N_SIZE_BLK - 1) / N_SIZE_BLK);	/* Start LB */
	edlb = (WORD)((region[1] + 1) / N_SIZE_BLK);				/* End LB + 1 */

	for (pb = N_RES_BLKS; pb < N_TOTAL_BLKS; pb++) {
		if (BlockMap[pb] >= stlb && BlockMap[pb] < edlb) {
			if (!erase_phy_block(pb)) bad_block(pb);	/* Erase the logical block */
		}
	}

	return RES_OK;
}



/*----------------------------------------------*/
/* Check if the block can be written well       */

static
int chk_block (	/* 0:Failed, 1:Successful */
	WORD blk,	/* Physical block to be checked */
	DWORD pat	/* Test pattern */
)
{
	DWORD *buf = SectorBuff[0];
	int i;


	for (i = 0; i < 132 * N_SIZE_BLK; i++)	/* Prepare a check pattern */
		buf[i] = pat;

	if (!write_sector(buf, (DWORD)blk * N_SIZE_BLK, N_SIZE_BLK))	/* Write it to the block */
		return 0;

	read_sector(buf, (DWORD)blk * N_SIZE_BLK, N_SIZE_BLK, 528);		/* Read-back the data */

	for (i = 0; i < 132 * N_SIZE_BLK; i++) {						/* Compare check pattern */
		if (buf[i] != pat) return 0;
	}

	return 1;
}



/*----------------------------------------------*/
/* Create physical format on the memory         */

static
DRESULT phy_format (
	int(*func)(WORD,WORD,WORD)	/* Pointer to call-back function for progress report */
)
{
	WORD pb, nb;


	nand_init();
	Init = nb = 0;

	for (pb = N_RES_BLKS; pb < N_TOTAL_BLKS; pb++) {	/* Check all blocks but reserved blocks */
		if (   !erase_phy_block(pb)			/* Check if the block has defect or not */
			|| !chk_block(pb, 0x55AA55AA)
			|| !erase_phy_block(pb)
			|| !chk_block(pb, 0xAA55AA55)
			|| !erase_phy_block(pb)
			) {
			bad_block(pb);	/* Mark 'bad-block' if the block is bad */
			nb++;
		}
		if (func && !func(pb - N_RES_BLKS + 1, N_TOTAL_BLKS - N_RES_BLKS, nb))	/* Report progress in format (processed, total, bad)*/
			return RES_ERROR;
		if (nb > N_SPARE_BLKS / 2) return RES_ERROR;	/* Too many bad blocks */
	}

	return RES_OK;
}



/*--------------------------------------------*/
/* Create ECC value of a 256 byte data block  */

static
DWORD create_ecc (		/* Returns 22 bit parity (b0:P0, b1:P0', b2:P1, b3:P1', b4:P2, ..., b21:P10') */
	const DWORD* blk	/* Pointer to the 256 byte data block to be calcurated */
)
{
	UINT i;
	DWORD d, r, par[22], *pp;

	/* Clear parity accumlator */
	for (i = 0; i < 22; par[i++] = 0) ;

	/* Calcurate parity */
	for (i = 0; i < 64; i++) {
		d = *blk++;		/* Get four bytes */
		pp = par;		/* Parity table */
		*pp++ ^= d & 0x55555555;		/* P0  */
		*pp++ ^= d & 0xAAAAAAAA;		/* P0' */
		*pp++ ^= d & 0x33333333;		/* P1  */
		*pp++ ^= d & 0xCCCCCCCC;		/* P1' */
		*pp++ ^= d & 0x0F0F0F0F;		/* P2  */
		*pp++ ^= d & 0xF0F0F0F0;		/* P2' */
		*pp++ ^= d & 0x00FF00FF;		/* P3  */
		*pp++ ^= d & 0xFF00FF00;		/* P3' */
		*pp++ ^= d & 0x0000FFFF;		/* P4  */
		*pp++ ^= d & 0xFFFF0000;		/* P4' */
		pp[i >> 0 & 1] ^= d; pp += 2;	/* P5, P5' */
		pp[i >> 1 & 1] ^= d; pp += 2;	/* P6, P6' */
		pp[i >> 2 & 1] ^= d; pp += 2;	/* P7, P7' */
		pp[i >> 3 & 1] ^= d; pp += 2;	/* P8, P8' */
		pp[i >> 4 & 1] ^= d; pp += 2;	/* P9, P9' */
		pp[i >> 5 & 1] ^= d;			/* P10, P10' */
	}

	/* Gather parity and pack it into a DWORD */
	for (i = r = 0; i < 22; i++) {
		d = par[i];
		d ^= d >> 16;
		d ^= d >> 8;
		d ^= d >> 4;
		d ^= d >> 2;
		d ^= d >> 1;
		r |= (d & 1) << i;
	}

	return r;
}


/*----------------------------------------------*/
/* Check and correct a 256 byte data block      */

static
UINT check_ecc (	/* ==0:No error, b0:A bit error in data, b1:A bit error in ECC, b2:Unrecoverable (2+ bits error) */
	DWORD* blk,		/* Pointer to the 256 byte data block to be checked */
	DWORD ecc		/* Pre-calcurated 22 bit ECC for the data block */
)
{
	UINT i, bit, ofs;


	ecc = (ecc ^ create_ecc(blk)) & 0x3FFFFF;	/* Compare ECC */

	if (!ecc) return 0;							/* Succeeded(0): No error */
	if (!(ecc & (ecc - 1))) return 1;			/* Succeeded(1): Single-bit error in the ECC */
	if (((ecc ^ (ecc >> 1)) & 0x155555) != 0x155555) return 4;	/* Failed(4): Multiple-bit error */

	/* Correct single-bit error in the data block */
	for (bit = 0, i = 1; i < 32; i <<= 1, ecc >>= 2) {
		if (ecc & 2) bit |= i;
	}
	for (ofs = 0, i = 1; i < 64; i <<= 1, ecc >>= 2) {
		if (ecc & 2) ofs |= i;
	}
	blk[ofs] ^= 1 << bit;

	return 2;	/* Succeeded(2): Single-bit error in the data was corrected */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS NAND_initialize (void)
{
	DWORD i, s, d[2], bb;


	nand_init();
	flush_block();

	/* Create block map table */
	for (bb = 0, i = N_RES_BLKS; i < N_TOTAL_BLKS; i++) {
		read_sector(d, i * N_SIZE_BLK, 1, 8);	/* Read a first two DWORDs of the spare area */
		s = d[0];
		if (d[1] != 0xFFFFFFFF || (s != 0xFFFFFFFF && (WORD)s != (WORD)(~s >> 16))) {	/* Not a blank block? */
			if (++bb >= N_SPARE_BLKS) break;	/* Too many bad blocks */
			s = 0xFFFFFFFE;
		}
		BlockMap[i] = (WORD)s;
	}
	nand_cmd(0x00);		/* Exit read mode 3 */

	Init = (bb < N_SPARE_BLKS) ? 1 : 0;
	BadBlocks = bb;
	LogBlock = 0xFFFF;
	BlockDirty = 0;

	return Init ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS NAND_status (void)
{
	return Init ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT NAND_read (
	BYTE* buff,		/* Data read buffer */
	DWORD lba,		/* Start sector number (LBA) */
	BYTE sc			/* Number of sectors to read */
)
{
	WORD lb;
	UINT s, i;


	if (!Init) return RES_NOTRDY;				/* Initialized? */
	if (lba + sc > MAX_LBA) return RES_PARERR;	/* Range check */

	while (sc) {
		lb = lba / N_SIZE_BLK;	/* Block to be read */
		if (lb != LogBlock) {	/* Load a new block if block is different */
			if (!flush_block()) return RES_ERROR;
			load_block(lb, 1);
		}
		do {
			i = lba % N_SIZE_BLK;	/* Sector offset in the block */
			/* Check ECC if the sector is live and not checked */
			if (SectorBuff[i][128] != 0xFFFFFFFF && !SectStat[i]) {
				s = check_ecc(&SectorBuff[i][0], SectorBuff[i][130]);
				if (s & 4) return RES_ERROR;
				if (s & 2) SectorBuff[i][130] = create_ecc(&SectorBuff[i][0]);
				if (s & 3) BlockDirty = 1; 
				s = check_ecc(&SectorBuff[i][64], SectorBuff[i][131]);
				if (s & 4) return RES_ERROR;
				if (s & 2) SectorBuff[i][131] = create_ecc(&SectorBuff[i][64]);
				if (s & 3) BlockDirty = 1; 
				SectStat[i] = 1;
			}
			Copy_al2un(buff, SectorBuff[i], 512);	/* Copy a sector to app buffer */
			buff += 512; lba++; sc--;
		} while (sc && (lba % N_SIZE_BLK));		/* Repeat until last sector or end of block */
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* Note: Must be synced after write process to flush the dirty block     */

DRESULT NAND_write (
	const BYTE* buff,	/* Data to be written */
	DWORD lba,			/* Start sector number (LBA) */
	BYTE sc				/* Number of sectors to write */
)
{
	WORD lb;
	UINT i;
	int load;


	if (!Init) return RES_NOTRDY;				/* Initialized? */
	if (lba + sc > MAX_LBA) return RES_PARERR;	/* Range check */

	while (sc) {
		lb = lba / N_SIZE_BLK;	/* Block to be written */
		i = lba % N_SIZE_BLK;	/* Sector offset in the block */
		if (lb != LogBlock) {	/* Load a new block if block is different */
			if (!flush_block()) return RES_ERROR;
			load = (i == 0 && sc >= N_SIZE_BLK) ? 0 : 1;	/* Do not read block if the entire block is to be overwritten */
			load_block(lb, load);
		}
		do {
			Copy_un2al(SectorBuff[i], buff, 512);					/* Copy app data to the sector */
			SectorBuff[i][130] = create_ecc(&SectorBuff[i][0]);		/* Create ECC 1 */
			SectorBuff[i][131] = create_ecc(&SectorBuff[i][64]);	/* Create ECC 2 */
			SectStat[i] = 1;
			buff += 512; i++; lba++; sc--;
		} while (sc && (lba % N_SIZE_BLK));	/* Repeat until last sector or end of block */
		BlockDirty = 1;
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT NAND_ioctl (
	BYTE ctl,
	void* buff
)
{
	switch (ctl) {
	case CTRL_SYNC:			/* Flush dirty block */
		if (!Init) return RES_NOTRDY;
		return flush_block() ? RES_OK : RES_ERROR;

	case GET_SECTOR_COUNT:	/* Get number of user sectors */
		*(DWORD*)buff = (DWORD)MAX_LBA;
		return RES_OK;

	case GET_BLOCK_SIZE:	/* Get erase block size */
		*(DWORD*)buff = N_SIZE_BLK;
		return RES_OK;

	case CTRL_ERASE_SECTOR:	/* Erase (force blanked) a sector group */
		if (!Init) return RES_NOTRDY;
		return blank_log_blocks(buff);

	case NAND_FORMAT:	/* Create physical format on the memory */
		return phy_format(buff);
	}

	return RES_PARERR;
}

