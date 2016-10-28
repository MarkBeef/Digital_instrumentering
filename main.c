#include <main.h>
#use I2C (master, sda = PIN_C4, scl = PIN_C3)

#define resolution 0.039
#define YELLOW_LED PIN_D1



void menu();
void CLRScreen ();
void PutCursor (int x, int y);
void CLRLine ();
void display_help();
int16 set_waittime();
void write_ext_eeprom(int16 address, BYTE data);
void write_int16_ext_eeprom(int16 address, int16 data);
void delete_eeprom(int16 *address);
int16 read_int16_ext_eeprom(int16 address);
BYTE read_ext_eeprom (int16 address);
void loggin(int16 *address, int16 data_av1, int16 data_av2);
void read_temperature_register(int8* data_high, int8* data_low, int16 wait_time);
void print_lcd(char* string_1, char* string_2, float value_1, float value_2);
void print_eeprom(int16 address);

int8 BCDtoBIN(int8 x);
int8 BINtoBCD(int8 x);

void main() {
   
   
   // Oscillator setup
   signed int N = 14;
   setup_oscillator(OSC_8MHZ, N);
   // Variable declartion
   int1 check = 1;
   int1 logging =0;
   signed int8 data_high;
   signed int8 data_low;
   signed int16 data;
   signed int16 data_av1;
   signed int16 data_av2;
   int16 count;
   float temperature1;
   float temperature2;
   signed int16 adc_voltage;
   int16 address = 0;
   char* temp1 = "TempA: ";
   char* temp2 = "TempD: ";
   int16 wait_time;
   
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
   
   // LCD setup
   lcd_init(); // initialize LCD
   lcd_clear(); // reset LCD
   
   
    menu();

   while(TRUE) { 
      
        if ( kbhit()) // en karakter er blevet overført til karakterbufferen i PIC'ens hardware
      {
         switch(getc()) // karakteren læses fra karakterbufferen, som tømmes
         {
            case 'h': 
               display_help();
                  
               break;
            case 'l' : 
               logging = 1; 
               CLRScreen();
               printf("Now Logging. Press s to stop.");
               break;
            case 's':
               logging = 0;
               printf("%16.0w", address);
               delay_ms(2000);
               CLRScreen();
               menu();
               break;
            case 't' : 
               wait_time = set_waittime(); 
               break;
            case 'd' : 
               CLRScreen();
               printf("Now Deleting - Please Wait");
               delete_eeprom(&address);
               CLRScreen();
               printf("Finished...");
               delay_ms(1000);
               menu();
               break;
            case 'p' : 
               print_eeprom(address); 
               break;
            default: break;
         }
      }
       
       if(logging) {
         
         adc_voltage = read_adc();
          
         // read from the temperature register.
         read_temperature_register(&data_high, &data_low, wait_time);
         
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
           
         
      }
      
   }
   
   
   }



}

   

   /* if ( kbhit()) // en karakter er blevet overført til karakterbufferen i PIC'ens hardware
      {
         switch(getc()) // karakteren læses fra karakterbufferen, som tømmes
         {
         case 'h': display_help(); break;
         case 'l' : logging = 1; break;
         case 's': logging = 0; break;
         case 't' : set_logging_interval(); break;
         default: break;
         } */
       
     


void delete_eeprom(int16 *address) {
   int16 i;
   int16 zero = 0;
   for(i=0; i <= 8192; i += 2){
      write_int16_ext_eeprom(i, zero);
   }
   *address = 0;
}


void write_int16_ext_eeprom(int16 address, int16 data)
{
int8 i;
for(i = 0; i < 2; ++i) write_ext_eeprom(address + i, *((int8 *)(&data) + i));
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

int16 read_int16_ext_eeprom(int16 address)
{
int8 i;
int16 data;
for(i = 0; i < 2; ++i) *((int8 *)(&data) + i) = read_ext_eeprom(address + i);
return(data);
}

void print_eeprom(int16 address) {
   int i;
   float data;
   CLRScreen();
   putcursor(1,1);
   printf("TempD:");
   putcursor(20,1);
   printf("TempA");
   for(i = 0; i <= address-4; i += 4) {
         putcursor(1,2+(i>>2));
         data = read_int16_ext_eeprom(i)/128.0;
         printf("%2.2f", data);
         putcursor(20,2+(i>>2));
         data = (float)read_int16_ext_eeprom(i+2)*resolution;
         printf("%2.2f", data);
         
      }
   
   
}


void loggin(int16 *address, int16 data_av1, int16 data_av2) {
   output_high(YELLOW_LED);
   write_int16_ext_eeprom(*address, data_av1);
   *address += 2;
   write_int16_ext_eeprom(*address, data_av2);
   *address += 2;
   output_low(YELLOW_LED);
}

void read_temperature_register(int8* data_high, int8* data_low, int16 wait_time) {
// read from the temperature register.
   int1 check = 1;
   i2c_start();
   check = i2c_write(0x91);
      if(!check) {
         *data_high = i2c_read(TRUE);
         *data_low = i2c_read(FALSE);
         delay_ms(200+wait_time);
      }
   i2c_stop();
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

////////////////////////////////////////////////////////////////////////////
//// Routines for VT -100 Terminal                                      ////
////                                                                    ////
//// void CLRScreen(). Clears the screen.                               ////
////                                                                    ////
//// void PutCursor(int x, int y). Sets the cursor.                     ////
////                                                                    ////
//// void CLRLine(). Clears the current line.                           ////
////                                                                    ////
////////////////////////////////////////////////////////////////////////////

void CLRScreen (){
   putc(0x1b); // <ESC> Clear display
   putc(0x5b); // [
   putc(0x32); // 2
   putc(0x4a); // J
   putc(0x1b); // <ESC> Reset cursor position 
   putc(0x5b); // [
   putc(0x30); // 0
   putc(0x3b); // ;
   putc(0x30); // 0
   putc(0x48); // H
}

void PutCursor (int x, int y) {
    int y1, y2, x1, x2 = 0;
   
    putc(0x1b); // <ESC>
    putc(0x5b); // [
   
    if(y>9) { // Testing to see if y > 9
     y2 = (y%10);
     y1 = (y-y2)/10;
     putc(48+y1);
     putc(48+y2);
    }
    else putc(48+y);
   
    putc(0x3b); // ;
    if(x>9) { // Testing to see if x > 9
     x2 = (x%10);
     x1 = (x-x2)/10;
     putc(48+x1);
     putc(48+x2);
    }
    else putc(48+x);
    putc(0x48); // H
}


void CLRLine (){
    putc (0x1b ); // <ESC>
    putc (0x5b ); // [
    putc (0x32 ); // 2
    putc (0x4b ); // K
}

void display_help() {
   CLRScreen();
   printf("Ey yo, you need help?");
   putCursor(1,1);
   printf("By pressing L you start the logging!");
   putCursor(1,2);
   printf("By pressing S you stop the logging!");
   putCursor(1,3);
   printf("Press C to print whats in the logger!");
   putCursor(1,4);
   printf("Pressing D erases the logger!");
   putCursor(1,5);
   printf("Press E will get your ass back to the main menu!");
   putCursor(1,6);
   printf("Pressing anything else will fuck your day up!!!");
}

int16 set_waittime() {
   CLRScreen();
   Printf("Enter the desired number for the waittime. Exit by pressing a");
   putCursor(0,1);
   int16 wTT = 0;
   int16 waitTime = 0;
   char whatsputin;
   while(TRUE) {
      if(kbhit()) {
         whatsputin = getc();
         wTT = whatsputin - 0x30;
         if( (wTT >= 0) && (wTT <= 0x09)) {
            waitTime *= 10;
            waitTime += wTT;
         }
         else if(whatsputin == 'a') {
            return waitTime;
            menu();
         }
      }
   }
}

void menu(){

CLRScreen();

putCursor(1,1);
printf("Press: h for help" );
putCursor(1,3);
printf("Press l to start logging" );
putCursor(1,5);
printf("Press s to stop logging" );
putCursor(1,7);
printf("Press t to set time between two loggings");
putcursor(1,9);
printf("Press p to print EEPROM");
putcursor(1,11);
printf("Press d to reset EEPROM");

}


set_real_timer(int8 year, int8 month, int8 date, int8 day, int8 hours, int8 minutes, int8 seconds) {
   
   int8 this_year = BINtoBCD(year);
   int8 this_month = BINtoBCD(month);
   int8 this_date = BINtoBCD(date);
   int8 this_day = BINtoBCD(day);
   int8 this_hours = BINtoBCD(hours);
   int8 this_minutes = BINtoBCD(minutes);
   int8 this_seconds = BINtoBCD(seconds);
   
   
   
   
}

int8 BCDtoBIN(int8 x) { return(((x>>4)*10)+(x%16)) }
int8 BINtoBCD(int8 x) { return(((x/10)<<4)+(x%10)) }
