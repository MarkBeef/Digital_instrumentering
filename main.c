#include <main.h>

// SPI pins declaration:
#define SCK PIN_C3
#define SDO PIN_C5
#define SDI PIN_C4
#define CS PIN_A5

// Global variables:
int1 check = 0; 
int16 in_data;
long overflow_count;

// timer function:
#int_timer1
void timer1_isr()
   {
   overflow_count++;
   }

// external interrupts used for calibration:
#int_ext
void signal_isr()
   {
      check = 1;
   
   }


// function initializations:
void write_float_eeprom(int16 address, float data);
float read_float_eeprom(int16 address);
void adc(int8 out_data1, int8 out_data2, float *voltage, float calibration);


void main()
{

   // Oscillator setup
   signed int N = 14;
   setup_oscillator(OSC_8MHZ, N);
   
   // SPI setup: 
   setup_spi( SPI_MASTER | SPI_L_TO_H | SPI_CLK_DIV_16 );
   
   // LCD setup
   lcd_init(); // initialize LCD
   lcd_clear(); // reset LCD
   
 
   // timer setup:
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_2);
   enable_interrupts(int_timer1);
   
   ext_int_edge(H_TO_L);
   enable_interrupts(int_ext);
   enable_interrupts(global);
   
   
   // Variable declaration:
   float voltage;
   int count = 0;
   float voltage_av = 0;
   float data;
   char buf[17];
   int8 out_data1 = 0x01;
   int8 out_data2 = 0xE0;
   
   // Get calibration data from eeprom:
   float calibration = read_float_eeprom(1);
   // setting CS high:
   output_high(CS); 
   
   
   // main loop:
   while(TRUE)
   {     
         
         // Convert and store data in voltage.
         adc(out_data1, out_data2, &voltage, calibration);
         
         
         // Calibration if the interrupt happens, then check = 1 and calibration is being made.
         if(check || count) {
            
            // count iterates, to start finding average:
            count++;
            // check is set to zero again.
            check = 0;
            
            // voltages are added together.
            voltage_av = voltage_av + (float)in_data;
            
            // when 16 samples are made calculate average:
            if(count == 16) {
               
               voltage_av /= 16;
               
               // 3900 is 3.9V. The voltage where we calibrate at. This is the calibration factor.
               data = 3900 / voltage_av;
               
               // store data in eeprom:
               write_float_eeprom( 1, data); 
               
               // calibration variable is made for future use on voltage samples.
               calibration = data;
               
               // printing the calibration factor.
               printf("Calibration: %10.6f \n ", calibration);
               
               // set average and count to zero again:
               count = 0;
               voltage_av = 0;
            }
         }
         
         // LCD only updates every 1 second to minimize seizure for user.
         if(overflow_count >= 16) {
            
            // Print Voltage to the LCD:
            disable_interrupts(global);
            lcd_clear();
            lcd_gotoxy( 1, 1);
            strcpy (buf, "Voltage: "); // Text to display on line 1
            lcd_print(buf);
            sprintf(buf," %10.3f ", voltage);  // place a string in buf.
            lcd_gotoxy( 1, 2);             // go to start of LCD line 2
            lcd_print(buf);   // write buf to LCD
            
            overflow_count = 0;
            enable_interrupts(global);
         }
      
   }

}


// function that writes to eeprom
void write_float_eeprom(int16 address, float data)
{
int8 i;
for( i =0; i < 4; i++) write_eeprom(address + i, *((int8 *)(&data)+i) );
}

// function that reads from eeprom:
float read_float_eeprom(int16 address)
{
int8 i;
float data;
for(i = 0; i < 4; ++i) *((int8 *)(&data) + i) = read_eeprom(address + i);
return data;
}


// function that convertes and stores data in the voltage variable.
void adc(int8 out_data1, int8 out_data2, float *voltage, float calibration) {
   
      output_low(CS);   
      SPI_read(out_data1);
      in_data = SPI_read(out_data2) << 8;
      in_data += SPI_read(0); 
      in_data &= 0x1FFF;
         
      output_high(CS);
      
      *voltage = (float)in_data*0.001*calibration;
   
   
}
