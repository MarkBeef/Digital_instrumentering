#include <main.h>
#include <stdlib.h>
#use rtos(timer=1, minor_cycle=100ms)
#use I2C (master, sda = PIN_C4, scl = PIN_C3)

#define resolution 0.039
#define YELLOW_LED PIN_D1


#task(rate=600ms, max = 20ms)
void key_task(); 

#task(rate=600ms, enabled=0)
void log_data_task();

#task(rate = 600ms, max = 20ms, enabled=0)
void delete_eeprom();

#task(rate = 600ms, enabled=0)
void print_eeprom();

#task(rate = 600ms, enabled=0)
void set_time_RTC();

#task(rate = 600ms, enabled=0)
void set_waittime();




void print_lcd(float value_1, float value_2);
void loggin(int16 data_av1, int16 data_av2, int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year);
void read_time_RTC(int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year);
void write_int16_ext_eeprom(int16 address, int16 data);
void write_ext_eeprom(int16 address, BYTE data);
BYTE read_ext_eeprom (int16 address);
int16 read_int16_ext_eeprom(int16 address);
void read_temperature_register(int8* data_high, int8* data_low);

void CLRScreen ();
void PutCursor (int x, int y);
void CLRLine ();

int8 BCDtoBIN(int8 x);
int8 BINtoBCD(int8 x);

void get_string(char* s, unsigned int8 max);
signed int8 get_Int8(void);
signed int16 get_Int16(void);

void menu();

unsigned int16 count=0;
int16 address = 0;
int16 wait_time = 1;
int1 flag = 0;
int8 min1,hours1,day1,date1,month1,year1;


void main()
{

   signed int N = 14;
   setup_oscillator(OSC_8MHZ, N);
   
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
   

   rtos_run();

}

void key_task()
{

if ( kbhit()) // en karakter er blevet overført til karakterbufferen i PIC'ens hardware
      {
         switch(getc()) // karakteren læses fra karakterbufferen, som tømmes
         {
            case 'l' : 
               rtos_enable(log_data_task);
               break;
            case 's':
               rtos_disable(log_data_task);
               break;
            case 't' : 
               rtos_disable(key_task);
               rtos_enable(set_waittime);
               break;
            case 'd' : 
               rtos_enable(delete_eeprom);
               break; 
            case 'p' : 
               rtos_enable(print_eeprom);
               break; 
            case 'c' :
               flag = 1;
               rtos_disable(key_task);
               rtos_enable(set_time_RTC);
               break;
            case 'm' :
               menu();
               break;
         }
      }
}

void log_data_task()
{
   rtos_await(++count == wait_time);
   count = 0;
   // variables:
   int8 data_high, data_low;
   // Time variables:
   int8 sec, min, hours, day, date, month, year;
   
   int16 data_ana = read_adc();
   
   // read from the temperature register.
   read_temperature_register(&data_high, &data_low);
         
   // add the data together.
   int16 data_dig =  make16(data_high,data_low);
   
   float temperature1 = data_dig / 128.0;
   float temperature2 = (float)data_ana * resolution;
   
   loggin(data_dig, data_ana, &sec, &min, &hours, &day, &date, &month, &year);
   
   print_lcd(temperature1, temperature2);
   
   if(flag == 0) {
      CLRScreen();
      putcursor(1,1);
      printf("TempD:");
      putcursor(20,1);
      printf("TempA");
      putcursor(30,1);
      printf("Time");
      putcursor(1,2);
      printf("%2.2f", temperature1);
      putcursor(20,2);
      printf("%2.2f", temperature2);
      putcursor(30,2);
      printf("%2.0w:%2.0w:%2.0w:%2.0w:%2.0w:%2.0w:%2.0w:",BCDtoBIN(sec),BCDtoBIN(min),BCDtoBIN(hours),BCDtoBIN(day),BCDtoBIN(date),BCDtoBIN(month),BCDtoBIN(year)); 
   }
}

void set_time_RTC()
{  
   flag = 1;
   CLRScreen();
   putcursor(1,1);
   printf("Set minutes: ");
   rtos_await(kbhit());
   min1 = BINtoBCD(get_Int8());
   putcursor(1,3);
   printf("Set hours: ");
   rtos_await(kbhit());
   hours1 = BINtoBCD(get_Int8());
   putcursor(1,5);
   printf("Set day: ");
   rtos_await(kbhit());
   day1 = BINtoBCD(get_Int8());
   putcursor(1,7);
   printf("Set date: ");
   rtos_await(kbhit());
   date1 = BINtoBCD(get_Int8());   
   putcursor(1,9);
   printf("Set month: ");
   rtos_await(kbhit());
   month1 = BINtoBCD(get_Int8());
   putcursor(1,11);
   printf("Set year: ");
   rtos_await(kbhit());
   year1 = BINtoBCD(get_Int8());
   flag = 0;
   
   i2c_start();
   i2c_write(0xD0);
   i2c_write(0x00);
   i2c_write(0);
   i2c_write(min1);
   i2c_write(hours1);
   i2c_write(day1);
   i2c_write(date1);
   i2c_write(month1);
   i2c_write(year1);
   i2c_stop();
   
   i2c_start();
   i2c_write(0xD0);
   i2c_write(0x00);
   i2c_stop();
   
   rtos_disable(set_time_RTC);
   rtos_enable(key_task);
}


void loggin(int16 data_av1, int16 data_av2, int8 *sec, int8 *min, int8 *hours, int8 *day, int8 *date, int8 *month, int8 *year) {
   output_high(YELLOW_LED);
   write_int16_ext_eeprom(address, data_av1);
   address += 2;
   write_int16_ext_eeprom(address, data_av2);
   address += 2;
   read_time_RTC(sec, min, hours, day, date, month, year);
   write_ext_eeprom(address, *sec);
   address += 1;
   write_ext_eeprom(address, *min);
   address += 1;
   write_ext_eeprom(address, *hours);
   address += 1;
   write_ext_eeprom(address, *day);
   address += 1;
   write_ext_eeprom(address, *date);
   address += 1;
   write_ext_eeprom(address, *month);
   address += 1;
   write_ext_eeprom(address, *year);
   address += 1;
   output_low(YELLOW_LED);
}

void print_eeprom()
{
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
      rtos_disable(print_eeprom);
}

void set_waittime() {
   CLRScreen();
   putcursor(1,1);
   Printf("Enter the desired seconds for the waittime.");
   wait_time = get_int16(); 
   rtos_disable(set_waittime);
   rtos_enable(key_task);
   count = 0;
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

void print_lcd(float value_1, float value_2) {
    
    char buf[6];
    lcd_clear();
    lcd_gotoxy( 1, 1);
    strcpy (buf, "TempD"); // Text to display on line 1
    lcd_print(buf);
    sprintf(buf," %2.2f C", value_1);  // place a string in buf.
    lcd_gotoxy( 8, 1);             // go to start of LCD line 2
    lcd_print(buf);   // write buf to LCD
    strcpy (buf, "TempA");
    lcd_gotoxy( 1, 2);
    lcd_print(buf);
    sprintf(buf," %2.2f C", value_2);
    lcd_gotoxy( 8,2);
    lcd_print(buf);
   
   
}



void delete_eeprom() {
   address = 0;
   rtos_disable(delete_eeprom);
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
putcursor(1,15);
printf("Address is currently: %4.0w", address);

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
