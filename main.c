#include <main.h> 

#define YELLOW_LED PIN_D1

int16 overflow_count;

char flag_int_count;

char timer_count_stop;


#int_ext
void signal_isr()
   {
   
   if(flag_int_count == 0) {
      set_timer1(0);
      overflow_count = 0;
      flag_int_count++;
      output_high(YELLOW_LED);
   }
   else {
      disable_interrupts(global);
      flag_int_count = 0;
      timer_count_stop++;
      output_low(YELLOW_LED);
   }
   
   
   }

#int_timer1
void timer1_isr()
   {
   overflow_count++;
   }


/*void signal_isr()
   {  
      output_high(YELLOW_LED);
      delay_ms(1000);
      output_low(YELLOW_LED);
   } */

void main()
{  
   // setup af interrupts og variabler
   int32 time;
   char buf[17]; // allocate a string of 16 characters
   signed int N = 14;
   lcd_init(); // initialize LCD
   lcd_clear(); // reset LCD
   
   output_low(YELLOW_LED);
   setup_oscillator(OSC_8MHZ, N);
   ext_int_edge(H_TO_L);
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_2);
   enable_interrupts(int_ext); 
   enable_interrupts(int_timer1);
   enable_interrupts(global);  
   port_b_pullups(TRUE);
   
   
   while(TRUE) {
      if(timer_count_stop) {
         lcd_clear();
         time = get_timer1();
         time = time + ((int32)overflow_count<<16);
         float time1 = (float)time/1000000;
         
         lcd_gotoxy( 1, 1);
         strcpy (buf, "Time (sec): "); // Text to display on line 1
         lcd_print(buf);
         sprintf(buf,"%10.3f",time1); // place a string in buf that shows time1 with three decimals
         lcd_gotoxy( 1, 2); // go to start of LCD line 2
         lcd_print(buf); // write buf to LCD
         timer_count_stop--;
         enable_interrupts(global);
      }
   
   }
}
