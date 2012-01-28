#ifndef _SCIF
#define _SCIF

#include "integer.h"

void scif_init (void);
int scif_test (void);
void scif_put (BYTE);
BYTE scif_get (void);

#endif

