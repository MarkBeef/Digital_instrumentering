#include <main.h>

#BYTE portc = 7

// Global position array for convenience.
BYTE const POSITION[4] = {0b0101, 0b1001, 0b1010, 0b0110};

// Initialize the step function.
void step(unsigned long delay1);


void main()
{
   // Microcontroller setup:
   signed int N = 14;
   setup_oscillator(OSC_8MHZ, N);
   set_tris_c(0xf0);
   
   // Variable Declaration:
   unsigned long delay = 5000;
   
   // This is the main loop:
   while(TRUE)
   {
      switch(getc())
         {  
            //Motor GO:
            case '1': 
               step(delay);
               printf("\n");
            break;
            
            // Decrease delay by 0.1ms
            case '2': 
               delay-= 100;
               printf("\n");
            break;
            
            // Increase delay by 0.1ms
            case '3': 
               delay+= 100;
               printf("\n");
            break;
            
            // Print the current delay
            case '4':
               
               printf("\n Delay is now: %luus \n", delay);
            break;
            
         }
   }

}

// Step function that drives the step motor 1 round. With adjustable delay.
void step(unsigned long delay1) {
      
      int i;
      
      for(i = 0; i <= 200; i++) {
         
        portc = POSITION[i&3]; 
        delay_us(delay1); 
         
      }
   
   
}
