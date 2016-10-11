#include <16F887.h>


#device ADC=10
#device ICD=TRUE

#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O

#use delay(internal=8MHz,clock_out)

#include "lcd16216.c"

#use rs232(debugger)
