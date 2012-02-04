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

#include <stdio.h>
#include "ff.h"



void die (		/* Stop with dying message */
	FRESULT rc	/* FatFs return value */
)
{
	printf("Failed with rc=%u.\n", rc);

	switch (rc)
	{

	case FR_EXIST:
		printf("File Already Exists\n");

	break;

	case FR_NO_FILE :
		printf("File Not Found\n");

	break;

	default:

		break;
	}

}


/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/
BYTE File_Exists(const TCHAR* path)
{
	FRESULT rc;				/* Result code */
	FILINFO info;
	rc = f_stat(path,&info);
	if (rc == FR_OK)
		{
		return 0x01;
		}

	if (rc) {
		die(rc);
	}

		return 0x00;
}


int main (void)
{
	FRESULT rc;				/* Result code */
	FATFS fatfs;			/* File system object */
	FIL fil;				/* File object */
	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	UINT bw, br, i;
	BYTE buff[128];




	f_mount(0, &fatfs);		/* Register volume work area (never fails) */

	printf("\nOpen a test file (message.txt).\n");
	rc = f_open(&fil, "MESSAGE.TXT", FA_READ);
	if (rc) die(rc);

	printf("\nType the file content.\n");
	for (;;) {
		rc = f_read(&fil, buff, sizeof(buff), &br);	/* Read a chunk of file */
		if (rc || !br) break;			/* Error or end of file */
		for (i = 0; i < br; i++)		/* Type the data */
			putchar(buff[i]);
	}
	if (rc) die(rc);

	printf("\nClose the file.\n");
	rc = f_close(&fil);
	if (rc) die(rc);

	printf("\nCreate a new file (hello.txt).\n");
	rc = f_open(&fil, "Hello.TXT", FA_WRITE | FA_CREATE_ALWAYS);
	if (rc) die(rc);

	printf("\nWrite a text data. (Hello world!)\n");
	rc = f_write(&fil, "Hello world!\r\n", 14, &bw);
	if (rc) die(rc);
	printf("%u bytes written.\n", bw);

	printf("\nClose the file.\n");
	rc = f_close(&fil);
	if (rc) die(rc);

	printf("\nCreate New Dir.\n");
	rc =f_mkdir ("NewDir");
	if (rc) die(rc);

	printf("\nCD into New Dir.\n");
	rc =f_chdir("NewDir");
	if (rc) die(rc);

	if (!File_Exists("NOFILE.LOG"))
	{
	printf("\nNOFILE.LOG Was not found. This is not any error.\n");
	}

	if (File_Exists("0001234.LOG"))
	{
	printf("\nDelete File (0001234.LOG).\n");
	rc = f_unlink("0001234.LOG");
	if (rc) die(rc);
	}

	printf("\nCreate a new file (0001234.LOG).\n");
	rc = f_open(&fil, "0001234.LOG", FA_WRITE | FA_CREATE_ALWAYS);
	if (rc) die(rc);

	printf("\nWrite a binary data.\n");

	for (i = 0; i < 0xFF; i++)
	{
		buff[0] = i;
		rc = f_write(&fil, buff, 1, &bw);
		if (rc)
			{
			die(rc);
			break;
			}
	}



	printf("\nClose the file.\n");
	rc = f_close(&fil);
	if (rc) die(rc);



	printf("\nOpen root directory.\n");
	rc = f_opendir(&dir, "");
	if (rc) die(rc);

	printf("\nDirectory listing...\n");
	for (;;) {
		rc = f_readdir(&dir, &fno);		/* Read a directory item */
		if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		if (fno.fattrib & AM_DIR)
			printf("   <dir>  %s\n", fno.fname);
		else
			printf("%8lu  %s\n", fno.fsize, fno.fname);
	}
	if (rc) die(rc);


	printf("\nCD into Root.\n");
	rc =f_chdir("..");
	if (rc) die(rc);

	printf("\nOpen root directory.\n");
	rc = f_opendir(&dir, "");
	if (rc) die(rc);

	printf("\nDirectory listing...\n");
	for (;;) {
		rc = f_readdir(&dir, &fno);		/* Read a directory item */
		if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		if (fno.fattrib & AM_DIR)
			printf("   <dir>  %s\n", fno.fname);
		else
			printf("%8lu  %s\n", fno.fsize, fno.fname);
	}
	if (rc) die(rc);

	printf("\nTest completed.\n");

}



/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/

DWORD get_fattime (void)
{
	return	  ((DWORD)(2010 - 1980) << 25)	/* Fixed to Jan. 1, 2010 */
			| ((DWORD)1 << 21)
			| ((DWORD)1 << 16)
			| ((DWORD)0 << 11)
			| ((DWORD)0 << 5)
			| ((DWORD)0 >> 1);
}
