#include <main.h>

// ADC resolution to get the measured voltage:
#define resolution 0.01953

long overflow_count;
#define duty_max 800.0

// Timer interrupt function:
#int_timer1
void timer1_isr()
   {
   overflow_count++;
   }


void regulate(unsigned int16 *duty, float result, float goal);

void main()
{

   // Oscillator setup
   signed int N = 13;
   setup_oscillator(OSC_8MHZ, N);
   
   // PWM initialization
   unsigned int16 duty = 400;
   setup_ccp1(CCP_PWM);
   setup_timer_2(T2_DIV_BY_1, 199, 1);
   set_pwm1_duty(duty);
   
   // ADC setup
   setup_adc(ADC_CLOCK_DIV_32);
   setup_adc_ports(sAN7 | VSS_VDD);
   set_adc_channel(7);
   
   // LCD setup:
   lcd_init(); // initialize LCD
   lcd_clear(); // reset LCD
   
   // variables used
   unsigned int16 adc_voltage;
   char buf[17];
   float result;
   float goal = 1.0;
   float duty_per = 0.0;
   
   // setup timer interrupts:
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_2);
   enable_interrupts(int_timer1);
   enable_interrupts(global);
   
   

   while(TRUE)
   {     
         // Reading the voltage across the the potentiometer which is steps
         adc_voltage = read_adc();
         // The voltage is now calculated with the resolution
         result = adc_voltage*resolution;
         // The duty-cycle is now adjusted to regulate the output voltage
         regulate(&duty, result, goal);
         // setting the duty cycle to the desired.
         set_pwm1_duty(duty);
        
          // The updating of the LCD. Updating every second.
          if(overflow_count >= 16) {
          
            // Rewriting the duty cycle in percentage:
            duty_per = ((long)duty)/duty_max * 100;
            disable_interrupts(global);
            
            // writing on the LCD:
            lcd_clear();
            lcd_gotoxy( 1, 1);
            strcpy (buf, "Duty: "); // Text to display on line 1
            lcd_print(buf);
            sprintf(buf,"%10.3f %%",duty_per);  // place a string in buf that shows time1 with three decimals
            lcd_gotoxy( 1, 2);             // go to start of LCD line 2
            lcd_print(buf);   // write buf to LCD
            
            // Set the overflow_count to zero again:
            overflow_count = 0;
            enable_interrupts(global);
         }
   }  
}

// Function that regulates the duty cycle. Keeping it at constant voltage.
void regulate(unsigned int16 *duty, float result, float goal) {

   // If the measured voltage is below the desired goal:
   if((*duty < 800) && (result < goal)) {
      (*duty)++;
   } 
   
   // If the measured voltage is higher than the desired goal:
   else if((*duty > 0) && (result > goal)) {
      (*duty)--;
   }
   // extra reasurence that duty cycles doesn't go above max.
   if(*duty >= 800) {
      duty = 800;
   }
}
