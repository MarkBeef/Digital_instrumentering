#include <assignment1.h>


#define YELLOW_LED PIN_A5

void main()
{

// Opgave 1

/*
   while(TRUE)
   {
      output_low(YELLOW_LED);
      delay_ms(500);
      output_high(YELLOW_LED);
      delay_ms(500);
   }
*/

/*
// Opgave 2

 while(TRUE){
      signed int N;
      for(N = -15; N <= 15; N++) {
         setup_oscillator(OSC_8MHZ, N);
         delay_ms(100);
         printf("%d \n", N );
         }
   }

*/

// Opgave 2 ekstra test af N. 
   signed int N = 14;
   setup_oscillator(OSC_8MHZ, N);
   while(TRUE){
      
      
      printf("Petter \n" );
      delay_ms(500);
   }





}
