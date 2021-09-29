#include <stdlib.h>
#include "attenuation.h"

#include "attenuation_tables.c"

const attenuation_t *attenuation_table[][7]=
  {          /* A        B          C          D          E          F          G */
    /* A */    {NULL,    att_AB,    att_AC,    att_AD,    att_AE,    att_AF,    att_AG},
    /* B */    {NULL,    NULL,      att_BC,    att_BD,    att_BE,    att_BF,    att_BG},
    /* C */    {NULL,    NULL,      NULL,      att_CD,    NULL,      NULL,      NULL},
    /* D */    {NULL,    NULL,      NULL,      NULL,      att_DE,    NULL,      NULL},
    /* E */    {NULL,    NULL,      NULL,      NULL,      NULL,      att_EF,    NULL},
    /* F */    {NULL,    NULL,      NULL,      NULL,      NULL,      NULL,      att_FG},
    /* G */    {NULL,    NULL,      NULL,      NULL,      NULL,      NULL,      NULL},
};
