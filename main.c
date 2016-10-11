   #include <main.h>
#use I2C (master, sda = PIN_C4, scl = PIN_C3)

#define resolution 0.039
#define YELLOW_LED PIN_D1
// Global variables:

int8 flag = 0; 
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
     flag++;
   }

void write_ext_eeprom(int16 address, BYTE data);
BYTE read_ext_eeprom (int16 address);
void write_int16_ext_eeprom(int16 address, int16 data);
int16 read_int16_ext_eeprom(int16 address);
void print_lcd(char* string_1, char* string_2, float value_1, float value_2);
void loggin (int16 *address, int16 data_av1, int16 data_av2);
void read_temperature_register(int8* data_high, int8* data_low);


void main()
{
   // Oscillator setup
   signed int N = 13;
   setup_oscillator(OSC_8MHZ, N);
   
   // LCD setup
   lcd_init(); // initialize LCD
   lcd_clear(); // reset LCD
   
   // timer setup:
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_2);
   enable_interrupts(int_timer1);
   
   ext_int_edge(H_TO_L);
   enable_interrupts(int_ext);
   enable_interrupts(global);
   port_b_pullups(TRUE);
   
   // i2c setup
   output_float (PIN_C4);
   output_float (PIN_C3);
   
   // Variable declaration
   int1 check = 1;
   char buf[17];
   signed int8 data_high;
   signed int8 data_low;
   signed int16 data;
   signed int16 data_av1;
   signed int16 data_av2;
   int16 count;
   float temperature1;
   float temperature2;
   signed int16 adc_voltage;
   int address = 0;
   char* temp1 = "TempD: ";
   char* temp2 = "TempA: ";
   
   
   
   // Setup of register4: 
   i2c_start ();
   i2c_write (0x90); // Bus address of the device, even address = write mode
   i2c_write (0x04); // Address for control / status register in device
   i2c_write (0x60); // Set the temperature resolution two 13 bits + sign
   i2c_stop ();
   
   // setup of register1
   i2c_start();
   i2c_write (0x90);
   i2c_write (0x01);
   i2c_write (0x70);
   i2c_stop ();
   
   // Lets point at the temperature register.
   i2c_start();
   i2c_write(0x90);
   i2c_write(0x00);
   i2c_stop();
   
   //Vref setup:
   setup_vref(0xE2);
   
   // ADC setup
   setup_adc (ADC_CLOCK_DIV_32);
   setup_adc_ports (sAN7 | VSS_VREF);
   set_adc_channel (7);
   
   while(TRUE)
   {
    
     if(flag > 2) {
      flag = 0;
     }
      switch(flag) {
      
      case 0 :
      
      adc_voltage = read_adc();
       
      read_temperature_register(&data_high, &data_low);
      
      // add the data together.
      data =  make16(data_high,data_low);
      
      // count number of samples
      count++;
      
      // add samples together.
      data_av1 += data;
      data_av2 += adc_voltage;
      
      
      
      // calculate average and show on display
      if(count >= 8){
         data_av1 = data_av1 >> 3;
         data_av2 = data_av2 >> 3;
         // store average in the float temperature:
         temperature1 = data_av1 / 128.0;
         temperature2 = (float)data_av2 * resolution;
         
         // print average on LCD:
         print_lcd(temp1, temp2, temperature1, temperature2);
         count = 0;
         data_av1 = 0;
         data_av2 = 0;
      }
      break;
      
      case 1 :
         
         //printf("Løkke 2 \n");
         adc_voltage = read_adc();
          
         // read from the temperature register.
         i2c_start();
         check = i2c_write(0x91);
         if(!check) {
            data_high = i2c_read(TRUE);
            data_low = i2c_read(FALSE);
            delay_ms(200);
         }
         i2c_stop();
         
         // add the data together.
         data =  make16(data_high,data_low);
         
         // count number of samples
         count++;
         
         // add samples together.
         data_av1 += data;
         data_av2 += adc_voltage;
         
         // calculate average and show on display
         if(count >= 8){
            data_av1 = data_av1 >> 3;
            data_av2 = data_av2 >> 3;
            // store average in the float temperature:
            temperature1 = data_av1 / 128.0;
            temperature2 = (float)data_av2 * resolution;
            
            // Logging:
            loggin(&address, data_av1, data_av2);
            
            // print average on LCD:
            print_lcd(temp1, temp2, temperature1, temperature2);
            
            count = 0;
            data_av1 = 0;
            data_av2 = 0;
           
      }break;
         
         
         
       case 2 :
         disable_interrupts(global);
         int16 i;
         // digital for loop:
         for( i=0; i <= address-2; i +=4)  {
         
         float temperatureD = read_int16_ext_eeprom(i)/128.0;
         
         printf("Digital thermometer %2.2f C \n", temperatureD);
         delay_ms(200);
         
         }
         
         for (i=2; i<= address; i += 4) {
            
            float temperatureA = (float)read_int16_ext_eeprom(i)*resolution;
            
            printf("Analog thermometer: %2.2f C \n", temperatureA);
            delay_ms(200);
         }
         
         
         flag = 0;
         enable_interrupts(global);
      
      
   break;

      }
   }
}


void write_ext_eeprom(int16 address, BYTE data)
{
   int8 status;
   i2c_start();
   i2c_write(0xa0); // i2c address for EEPROM, write mode
   i2c_write((address>>8)&0x1f); // MSB of data address, max 8 kB
   i2c_write(address); // LSB of data address
   i2c_write(data); // data byte written to EEPROM
   i2c_stop();
   // wait until EEPROM has finished writing
   i2c_start(); // restart
   status = i2c_write(0xa0); // get acknowledge from EEPROM
   while(status == 1) // no acknowledge received from EEPROM, so still busy
   {
   i2c_start();
   status=i2c_write(0xa0); // repeat while busy
   }
   i2c_stop();
   }
   
BYTE read_ext_eeprom (int16 address) {
   BYTE data;
   i2c_start();
   i2c_write(0xa0); // i2c address for EEPROM, write mode
   i2c_write((address>>8)&0x1f); // MSB of data address, max 8kB
   i2c_write(address); // LSB of data address
   i2c_start(); // Restart
   i2c_write(0xa1); // i2c address for EEPROM, read mode
   data=i2c_read(0); // read byte, send NACK
   i2c_stop();
   return(data);
}

void write_int16_ext_eeprom(int16 address, int16 data)
{
int8 i;
for(i = 0; i < 2; ++i) write_ext_eeprom(address + i, *((int8 *)(&data) + i));
}

int16 read_int16_ext_eeprom(int16 address)
{
int8 i;
int16 data;
for(i = 0; i < 2; ++i) *((int8 *)(&data) + i) = read_ext_eeprom(address + i);
return(data);
}

void print_lcd(char* string_1, char* string_2, float value_1, float value_2) {
    
    char buf[17];
    lcd_clear();
    lcd_gotoxy( 1, 1);
    strcpy (buf, string_1); // Text to display on line 1
    lcd_print(buf);
    sprintf(buf," %2.2f C", value_1);  // place a string in buf.
    lcd_gotoxy( 8, 1);             // go to start of LCD line 2
    lcd_print(buf);   // write buf to LCD
    strcpy (buf, string_2);
    lcd_gotoxy( 1, 2);
    lcd_print(buf);
    sprintf(buf," %2.2f C", value_2);
    lcd_gotoxy( 8,2);
    lcd_print(buf);
   
   
}


void loggin (int16 *address, int16 data_av1, int16 data_av2) {
   output_high(YELLOW_LED);
   write_int16_ext_eeprom(address, data_av1);
   *address += 2;
   write_int16_ext_eeprom(address, data_av2);
   *address += 2;
   output_low(YELLOW_LED);
}

void read_temperature_register(int8* data_high, int8* data_low) {
// read from the temperature register.
   int1 check = 1;
   i2c_start();
   check = i2c_write(0x91);
      if(!check) {
         *data_high = i2c_read(TRUE);
         *data_low = i2c_read(FALSE);
         delay_ms(200);
      }
   i2c_stop();
}
