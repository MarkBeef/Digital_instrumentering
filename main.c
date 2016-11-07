#include <main.h>
#include <stdlib.h>
#use I2C (master, sda = PIN_C4, scl = PIN_C3)

// The resolution conversion constant for the ADC
#define resolution 0.039
#define YELLOW_LED PIN_D1

//////////////////////////////////////////////////////
//// These are the functions used in the program. //// 
//////////////////////////////////////////////////////

// This function sets up the menu, which is also the help menu.
void menu();

// Escape codes for easier communication with the terminal emulator
void CLRScreen ();
void PutCursor (int x, int y);
void CLRLine ();

void display_help();

// With this function it is possible to set the time between logs
int16 set_waittime();

// These functions are used to write and read from the external EEPROM. void write_ext_eeprom(int16 address, BYTE data) and
// BYTE read_ext_eeprom (int16 address) are include in void write_int16_ext_eeprom(int16 address, int16 data) and 
// int16 read_int16_ext_eeprom(int16 address). This makes it possible to write and read two bytes at a time.
void write_ext_eeprom(int16 address, BYTE data);
void write_int16_ext_eeprom(int16 address, int16 data);
int16 read_int16_ext_eeprom(int16 address);
BYTE read_ext_eeprom (int16 address);

// This functions deletes all the contents of the external EEPROM and sets the address to zero.
void delete_eeprom(int16 *address);

// This function makes use of the int16 read_int16_ext_eeprom(int16 address) function and prints all the contents of the external
// EEPROM up to address.
void print_eeprom(int16 address);

// This function reads the calendar from the RTC. The reason why it takes pointers as variables is because the time variables are 
// variables in the main. The reason they are that is that troubleshooting was made easy.
void read_time_RTC(int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year);

// When this function is called it starts to log the temperatures from the digital and analog thermometer and the time from the RTC.
// It makes use of the read_Time_RTC function which is why it needs pointers to the time variables.
void loggin(int16 *address, int16 data_av1, int16 data_av2, int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year);

//This function reads the temperature from the digital thermometer. Wait_time is needed to set the time between two logs.
void read_temperature_register(int8* data_high, int8* data_low, int16 wait_time);

// Is used for the LCD module
void print_lcd(char* string_1, char* string_2, float value_1, float value_2);

// The RTC saves the calendar in the BIN format.
int8 BCDtoBIN(int8 x);
int8 BINtoBCD(int8 x);

// set_real_timer makes it able to set the RTC and includes all the I2C protocol. set_time_menu sets up a user friendly menu and 
// includes the inputs from the emulator terminal
void set_real_timer(int8 year, int8 month, int8 date, int8 day, int8 hours, int8 minutes, int8 seconds);
void set_time_menu();

// Prints the contents of the RTC. This function makes use of read_time_RTC
void print_RTC(int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year);

// These functions are used to receive the input from the emulator terminal.
void get_string(char* s, unsigned int8 max);
signed int8 get_Int8(void);
signed int16 get_Int16(void);



void main() {
   
   // Oscillator setup
   signed int N = 14;
   setup_oscillator(OSC_8MHZ, N);
   
   // Variable declartion
   int1 check = 1;
   int1 logging = 0;       // This is used 
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
   int16 wait_time = 0;
   
   int8 sec;
   int8 min;
   int8 hours;
   int8 day;
   int8 date;
   int8 month;
   int8 year;
   
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
            case 'c' :
               set_time_menu();
               break;
            case 'q' :
               print_RTC(&sec, &min, &hours, &day, &date, &month, &year); break;
            default: menu(); break;
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
            loggin(&address, data_av1, data_av2, &sec, &min, &hours, &day, &date, &month, &year);
            
            // print average on LCD:
            print_lcd(temp1, temp2, temperature1, temperature2);
            
            count = 0;
            data_av1 = 0;
            data_av2 = 0;
            delay_ms(wait_time);
           
         
      }
      
   }
   
   
   }



}

   

       
     


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
   int8 time;
   CLRScreen();
   putcursor(1,1);
   printf("TempD:");
   putcursor(20,1);
   printf("TempA");
   putcursor(30,1);
   printf("Time");
   for(i = 0; i <= address-11; i += 11) {
         putcursor(1,2+(i/11));
         data = read_int16_ext_eeprom(i)/128.0;
         printf("%2.2f", data);
         putcursor(20,2+(i/11));
         data = (float)read_int16_ext_eeprom(i+2)*resolution;
         printf("%2.2f", data);
         putcursor(30,2+(i/11));
         time = BCDtoBIN(read_ext_eeprom(i+4));  // seconds
         printf("%2.0w:",time);
         time = BCDtoBIN(read_ext_eeprom(i+5)); // minutes
         printf("%2.0w:",time);
         time = BCDtoBIN(read_ext_eeprom(i+6)); // hours
         printf("%2.0w ",time);
         time = BCDtoBIN(read_ext_eeprom(i+7)); // day
         printf("%2.0w:",time);
         time = BCDtoBIN(read_ext_eeprom(i+8)); // date
         printf("%2.0w:",time);
         time = BCDtoBIN(read_ext_eeprom(i+9)); // month
         printf("%2.0w:",time);
         time = BCDtoBIN(read_ext_eeprom(i+10)); // year
         printf("%2.0w:",time);

      }
   
   
}


void loggin(int16 *address, int16 data_av1, int16 data_av2, int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year) {
   output_high(YELLOW_LED);
   write_int16_ext_eeprom(*address, data_av1);
   *address += 2;
   write_int16_ext_eeprom(*address, data_av2);
   *address += 2;
   read_time_RTC(sec, min, hours, day, date, month, year);
   write_ext_eeprom(*address, *sec);
   *address += 1;
   write_ext_eeprom(*address, *min);
   *address += 1;
   write_ext_eeprom(*address, *hours);
   *address += 1;
   write_ext_eeprom(*address, *day);
   *address += 1;
   write_ext_eeprom(*address, *date);
   *address += 1;
   write_ext_eeprom(*address, *month);
   *address += 1;
   write_ext_eeprom(*address, *year);
   *address += 1;
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
         delay_ms(200);
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
   putcursor(1,1);
   int16 WT;
   Printf("Enter the desired number for the waittime. Exit by pressing a ");
   WT = get_int16();
   
   menu();
   
   return WT;
   
}

void menu(){

CLRScreen();

putCursor(1,1);
printf("Press h for help" );
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
putcursor(1,13);
printf("Press c to edit time");
putcursor(1,15);
printf("Press q to print RTC");

}


void set_real_timer(int8 year, int8 month, int8 date, int8 day, int8 hours, int8 minutes, int8 seconds) {
   
   int8 this_year = BINtoBCD(year);
   int8 this_month = BINtoBCD(month);
   int8 this_date = BINtoBCD(date);
   int8 this_day = BINtoBCD(day);
   int8 this_hours = BINtoBCD(hours);
   int8 this_minutes = BINtoBCD(minutes);
   int8 this_seconds = BINtoBCD(seconds);
   
   i2c_start();
   i2c_write(0xD0);
   i2c_write(0x00);
   i2c_write(this_seconds);
   i2c_write(this_minutes);
   i2c_write(this_hours);
   i2c_write(this_day);
   i2c_write(this_date);
   i2c_write(this_month);
   i2c_write(this_year);
   i2c_stop();
   
   i2c_start();
   i2c_write(0xD0);
   i2c_write(0x00);
   i2c_stop();
   
   
   
}

void set_time_menu() {
   
   CLRScreen();
   
   putcursor(1,1);
   printf("Set minutes: ");
   int8 minutes = get_Int8();
   putcursor(1,3);
   printf("Set hours: ");
   int8 hours = get_Int8();
   putcursor(1,5);
   printf("Set day: ");
   int8 day = get_Int8();
   putcursor(1,7);
   printf("Set date: ");
   int8 date = get_Int8();   
   putcursor(1,9);
   printf("Set month: ");
   int8 month = get_Int8();
   putcursor(1,11);
   printf("Set year: ");
   int8 year = get_Int8();
   putcursor(1,13);
   
   set_real_timer(year, month, date, day, hours, minutes, 0);
   delay_ms(100);
   menu();
   
   
}

void read_time_RTC(int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year) {
   
   i2c_start();
   i2c_write(0xD1);
   *sec = i2c_read(TRUE);
   *min = i2c_read(TRUE);
   *hours = i2c_read(TRUE);
   *day = i2c_read(TRUE);
   *date = i2c_read(TRUE);
   *month = i2c_read(TRUE);
   *year = i2c_read(TRUE);
   i2c_read(FALSE);
   i2c_stop();
   
}

void print_RTC(int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year) {
   CLRScreen();
   read_time_RTC(sec, min, hours, day, date, month, year);
   putcursor(1,1);
   printf("Here is the time in BCD: ");
   putcursor(1,2);
   printf("%2.0w: %2.0w: %2.0w: %2.0w: %2.0w: %2.0w: %2.0w: ", *sec, *min, *hours, *day, *date, *month, *year);
   putcursor(1,10);
   printf("Here is the time in BIN: ");
   putcursor(1,11);
   printf("%2.0w: %2.0w: %2.0w: %2.0w: %2.0w: %2.0w: %2.0w: ", BCDtoBin(*sec), BCDtoBin(*min), BCDtoBin(*hours), BCDtoBin(*day), BCDtoBin(*date), BCDtoBin(*month), BCDtoBin(*year));
   
   if(kbhit()) {
   menu();
   }
}

int8 BCDtoBIN(int8 x) {return(((x>>4)*10)+(x%16)); }
int8 BINtoBCD(int8 x) {return(((x/10)<<4)+(x%10)); }

void get_string(char* s, unsigned int8 max) {
   unsigned int8 len;
   char c;

   max-=2;
   len=0;
   do {
     c=getc();
     if(c==8) {  // Backspace
        if(len>0) {
          len--;
          putc(c);
          putc(' ');
          putc(c);
        }
     } else if ((c>=' ')&&(c<='~'))
       if(len<=max) {
         s[len++]=c;
         putc(c);
       }
   } while(c!=13);
   s[len]=0;
}

signed int8 get_Int8(void)
{
  char s[5];
  signed int8 i;

  get_string(s, sizeof(s));

  i=atoi(s);
  return(i);
}

signed int16 get_Int16(void)
{
  char s[7];
  signed int16 l;

  get_string(s, sizeof(s));
  l=atol(s);
  return(l);
}

