#include <main.h>
#include <math.h>

// Initialize the I2C
#use I2C (master, sda = PIN_C4, scl = PIN_C3)

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR 0x3C // with pins SA0=0, SA1=0

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS 0x00
#define FXOS8700CQ_WHOAMI 0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1 0x2A
#define FXOS8700CQ_M_CTRL_REG1 0x5B
#define FXOS8700CQ_M_CTRL_REG2 0x5C
#define FXOS8700CQ_M_CTRL_REG3 0x5D
#define FXOS8700CQ_WHOAMI_VAL 0xC7
#define FXOS8700CQ_M_DR_STATUS 0x32

// Constants
#define offset_x 228
#define offset_y -77
#define offset_z 403

// number of bytes to be read from the FXOS8700CQ
#define FXOS8700CQ_READ_LEN 13 // status plus 6 channels = 13 bytes


// Struct used for raw data output of FXOS8700CQ
typedef struct {
   int16 x;
   int16 y;
   int16 z;
} SRAWDATA;

// Roll, pitch, yaw
float phi, theta, psi;


/////////////////////
///// Functions /////
/////////////////////


// Function for calcultion of the angles
void compass(int16 Bx,  int16 By, int16 Bz, int16 Gx, int16 Gy, int16 Gz);

// Used for initializing the
int1 init_FXOS8700CQ();
// Initializes while also setting the 
int1 init2_FXOS8700CQ();

// Configures the FXOS8700CQ to remove offset
int1 configure_FXOS8700CQ();

// Reads accelerometer and magnetometer data from registers 0x01 - 0x06 and
// 0x33 - 0x38
int1 readAccelMagnData(SRAWDATA *pAccelData, SRAWDATA *pMagnData);

// Universal full read and write with I2C peripherals
int1 s_i2c_write(int8 slave_address, int8 slave_register, int8 data);
int1 s_i2c_read(int8 slave_address, int8 slave_register, int8* data, int8 read_length);

// Used for test
void test();
// Finds addresses on the I2C bus
void test2();

// ANSI escape codes
void CLRScreen();
void PutCursor(int x, int y);
void CLRLine();

// Arctang
float atan22(float y,float x);


void main() {
   
   int8 data_ready;
   /*int8 offset[6];
   int16 offset_x1, offset_y1, offset_z1;*/
   
   // Oscillator has to be set up
   setup_oscillator(OSC_8MHZ, 14);
   // SDA(PIN_C4) and SCL(PIN_C3) have to be open collector
   output_float(PIN_C4);
   output_float(PIN_C3);
   
   
   CLRScreen();
   SRAWDATA acceleration;
   SRAWDATA magneticFlux;
   init2_FXOS8700CQ();
   
   while(TRUE) {  
      s_i2c_read(FXOS8700CQ_SLAVE_ADDR,FXOS8700CQ_M_DR_STATUS,&data_ready,1);
      /*s_i2c_read(FXOS8700CQ_SLAVE_ADDR,0x01,&accel_x_MSB,1);
      s_i2c_read(FXOS8700CQ_SLAVE_ADDR,0x02,&accel_x_LSB,1);
      delay_ms(1000);
      accel_x = make16(accel_x_MSB, accel_x_LSB);
      PutCursor(1,2);
      printf("Accelerometer x-component: %Ld", accel_x);*/
      
      if( data_ready && 0x80 ) {
         CLRScreen();
         readAccelMagnData(&acceleration, &magneticFlux);
         /*// TEST TO SEE MAX AND MIN VALUES OF THE MAGNETIC OFFSET
         s_i2c_read(FXOS8700CQ_SLAVE_ADDR,0x3F,&offset[0],6);
         offset_x1 = make16(offset[0], offset[1]);
         offset_y1 = make16(offset[2], offset[3]);
         offset_z1 = make16(offset[4], offset[5]);
         PutCursor(1,2);
         printf("Offset_x: %Ld", offset_x1);
         PutCursor(1,3);
         printf("Offset_y: %Ld", offset_y1);
         PutCursor(1,4);
         printf("Offset_z: %Ld", offset_z1);*/
         
         // ACTUAL CODE
         compass(magneticFlux.x, magneticFlux.y, magneticFlux.z, acceleration.x, acceleration.y, acceleration.z);
         PutCursor(1,1);
         printf("Roll: %1.5f", phi*180/Pi);
         PutCursor(1,2);
         printf("Pitch: %1.5f", theta*180/Pi);
         PutCursor(1,3);
         printf("Yaw: %1.5f", psi*180/Pi);
         PutCursor(1,4);
         printf("Direction: ");
         if((5.89 < psi && psi <= 6.29)  ||  ( 0 <= psi && psi <= 0.39 )) {
            printf("N");
         }
         else if(0.39 < psi && psi <= 1.18) {
            printf("NW");
         }
         else if(1.18 < psi && psi <= 1.96) {
            printf("W");
         }
         else if(1.96 < psi && psi <= 2.75) {
            printf("SW");
         }
         else if(2.75 < psi && psi <= 3.53) {
            printf("S");
         }
         else if(3.53 < psi && psi <= 4.32) {
            printf("SE");
         }
         else if(4.32 < psi && psi <= 5.11) {
            printf("E");
         }
         else if(5.11< psi && psi <= 5.89) {
            printf("NE");
         }
         
         
         
         // RAW DATA
         /*PutCursor(20,1);
         printf("Magnetometer x-component: %Ld", magneticFlux.x);
         PutCursor(20,2);
         printf("Magnetometer y-component: %Ld", magneticFlux.y);
         PutCursor(20,3);
         printf("Magnetometer z-component: %Ld", magneticFlux.z);
         PutCursor(20,4);
         printf("Accelerometer x-component: %Ld", acceleration.x);
         PutCursor(20,5);
         printf("Accelerometer y-component: %Ld", acceleration.y);
         PutCursor(20,6);
         printf("Accelerometer z-component: %Ld", acceleration.z);*/
      }
   }

}


// Function for calcultion of the angles
void compass(int16 Bx,  int16 By, int16 Bz, int16 Gx, int16 Gy, int16 Gz) {
   
   float bxp = (signed int16)Bx;
   float byp = (signed int16)By;
   float bzp = (signed int16)Bz;
   float gxp = (signed int16)Gx;
   float gyp = (signed int16)Gy;
   float gzp = (signed int16)Gz;
  
   
   // Calculating the roll angle Phi radians:
   if(gzp == 0 && gyp < 0) {
   
      phi = Pi/2;   // maybe check this later...
      
   } else if(gzp == 0 && gyp > 0) {
      
      phi = -PI/2;  // maybe.. check this later...
      
   } else {
     phi = atan2(gyp,gzp);
   }  
   if(phi == 180) {
      phi = 0;
   }
   
   
   //De-rotating by the roll-angle:
   float bfy = byp*cos(phi) - bzp*sin(phi);
   bzp = byp*sin(phi) + bzp*cos(phi);
   gzp = gyp*sin(phi) + gzp*cos(phi);
   
   // calculating pitch angle Theta in radians:
   
   /*
   if( Gx > 0) {
      
      theta = Pi/2;
         
   } else if(Gx < 0) {
      
      theta = -Pi/2;
      
   } else {
   */
    //  float denominator = (float)gyp*sin(phi) + (float)gzp*cos(phi);
   theta = atan( gxp*(-1)/( gzp )) + 0.06;
   
   if(theta > Pi/2) {
      theta = Pi - theta;
   }
   else if(theta < -Pi/2) {
      theta = - Pi - theta;
   }
   
   //}
   
   // De-rotate by pitch angle:
   
   float bfx = bxp*cos(theta) + bzp*sin(theta);
   float bfz = -bxp*sin(theta) + bzp*cos(theta);
   
   // Calculating yaw:
   
   if(bfz == 0) {
      psi = -Pi/2;
   }
   
   psi = atan22( (-1)*bfy , bfx );
   
}



// read status and the three channels of accelerometer and
// magnetometer data from
// FXOS8700CQ (13 bytes)
int1 readAccelMagnData(SRAWDATA *pAccelData, SRAWDATA *pMagnData) {
   int8 Buffer[13]; // read buffer
   // read FXOS8700CQ_READ_LEN=13 bytes (status byte and the six channels of data)
   if(s_i2c_read(FXOS8700CQ_SLAVE_ADDR, 0x01, &Buffer, 12)) {
      // copy the 14 bit accelerometer byte data into 16 bit words
      pAccelData->x = (((int16)Buffer[0] << 8) | Buffer[1]);
      pAccelData->y = (((int16)Buffer[2] << 8) | Buffer[3]);
      pAccelData->z = (((int16)Buffer[4] << 8) | Buffer[5]);
      if(pAccelData->x & 0x8000) {
         pAccelData->x *= -1;
         pAccelData->x >>= 2;
         pAccelData->x *= -1;
      }
      else {
         pAccelData->x >>= 2;
      }
      pAccelData->x -= 140;
      if(pAccelData->y & 0x8000) {
         pAccelData->y *= -1;
         pAccelData->y >>= 2;
         pAccelData->y *= -1;
      }
      else {
         pAccelData->y >>= 2;
      }
      pAccelData->y += 110;
      if(pAccelData->z & 0x8000) {
         pAccelData->z *= -1;
         pAccelData->z >>= 2;
         pAccelData->z *= -1;
      }
      else {
         pAccelData->z >>= 2;
      }
      pAccelData->z -= 110;
      pAccelData->x &= 0xFFE0;
      pAccelData->y &= 0xFFE0;
      pAccelData->z &= 0xFFE0;
      // copy the magnetometer byte data into 16 bit words
      pMagnData->x = ((int16)Buffer[6] << 8) | Buffer[7];
      pMagnData->y = ((int16)Buffer[8] << 8) | Buffer[9];
      pMagnData->z = ((int16)Buffer[10] << 8) | Buffer[11];
   }
   else {
      return 0; // Returns if read not succesful
   }
   return 1;  // Returns if read succesful
}




// Sets up the FXOS8700CQ for 200Hz hybrid mode meaning that both accelerometer
// and magnetometer data are provided at the 200Hz rate. Also has fixed offset for 
// magnetometer registers
int1 init2_FXOS8700CQ() {
   int8 databyte = 0x00;  // Byte used for setup
   // write 0000 0000 = 0x00 to accelerometer control register 1 to place FXOS8700CQ into standby
   // [7-1] = 0000 000
   // [0]: active=0
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   // write 0001 1111 = 0x1F to magnetometer control register 1
   // [7]: m_acal=0: auto calibration disabled
   // [6]: m_rst=0: no one-shot magnetic reset
   // [5]: m_ost=0: no one-shot magnetic measurement
   // [4-2]: m_os=111=7: 8x oversampling (for 200Hz) to reduce magnetometer noise
   // [1-0]: m_hms=11=3: select hybrid mode with accel and magnetometer active
   databyte = 0x1F;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   // write 0011 0000 = 0x20 to magnetometer control register 2
   // [7]: reserved
   // [6]: reserved
   // [5]: hyb_autoinc_mode=1 to map the magnetometer registers to follow the
   // accelerometer registers
   // [4]: m_maxmin_dis=1 to disable default min/max latching 
   // [3]: m_maxmin_dis_ths=0
   // [2]: m_maxmin_rst=0
   // [1-0]: m_rst_cnt=00 to enable magnetic reset each cycle
   databyte = 0x30;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG2, databyte) != 1) {
      return 0;
   }
   // Sets the offsets
   databyte = offset_x >> 7;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, 0x3F, databyte) != 1) {
      return 0;
   }
   databyte = (offset_x << 1) & 0xFF;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, 0x40, databyte) != 1) {
      return 0;
   }
   databyte = offset_y >> 7;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, 0x41, databyte) != 1) {
      return 0;
   }
   databyte = (offset_y << 1) & 0xFF;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, 0x42, databyte) != 1) {
      return 0;
   }
   databyte = offset_z >> 7;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, 0x43, databyte) != 1) {
      return 0;
   }
   databyte = (offset_z << 1) & 0xFF;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, 0x44, databyte) != 1) {
      return 0;
   }
   // write 0000 0001 = 0x01 to XYZ_DATA_CFG register
   // [7]: reserved
   // [6]: reserved
   // [5]: reserved
   // [4]: hpf_out=0
   // [3]: reserved
   // [2]: reserved
   // [1-0]: fs=01 for accelerometer range of +/-4g range 
   databyte = 0x01;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_XYZ_DATA_CFG, databyte) != 1) {
      return 0;
   }
   // write 0011 1101 = 0x3D to accelerometer control register 1
   // [7-6]: aslp_rate=00
   // [5-3]: dr=111 for 0,7813Hz data rate (when in hybrid mode)
   // [2]: lnoise=1 for low noise mode
   // [1]: f_read=0 for normal 16 bit reads
   // [0]: active=1 to take the part out of standby and enablesampling
   databyte = 0x3D;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   return 1;
}

// Sets up the FXOS8700CQ for 200Hz hybrid mode meaning that both accelerometer
// and magnetometer data are provided at the 200Hz rate
int1 init_FXOS8700CQ() {
   int8 databyte = 0x00;  // Byte used for setup
   // write 0000 0000 = 0x00 to accelerometer control register 1 to place FXOS8700CQ into standby
   // [7-1] = 0000 000
   // [0]: active=0
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   // write 1001 1111 = 0x1F to magnetometer control register 1
   // [7]: m_acal=1: auto calibration disabled
   // [6]: m_rst=0: no one-shot magnetic reset
   // [5]: m_ost=0: no one-shot magnetic measurement
   // [4-2]: m_os=111=7: 8x oversampling (for 200Hz) to reduce magnetometer noise
   // [1-0]: m_hms=11=3: select hybrid mode with accel and magnetometer active
   databyte = 0x9F;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   // write 0010 0000 = 0x20 to magnetometer control register 2
   // [7]: reserved
   // [6]: reserved
   // [5]: hyb_autoinc_mode=1 to map the magnetometer registers to follow the
   // accelerometer registers
   // [4]: m_maxmin_dis=0 to retain default min/max latching even though not used
   // [3]: m_maxmin_dis_ths=0
   // [2]: m_maxmin_rst=0
   // [1-0]: m_rst_cnt=00 to enable magnetic reset each cycle
   databyte = 0x20;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG2, databyte) != 1) {
      return 0;
   }
   // write 0000 0001 = 0x01 to XYZ_DATA_CFG register
   // [7]: reserved
   // [6]: reserved
   // [5]: reserved
   // [4]: hpf_out=0
   // [3]: reserved
   // [2]: reserved
   // [1-0]: fs=01 for accelerometer range of +/-4g range 
   databyte = 0x01;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_XYZ_DATA_CFG, databyte) != 1) {
      return 0;
   }
   // write 0011 1101 = 0x3D to accelerometer control register 1
   // [7-6]: aslp_rate=00
   // [5-3]: dr=111 for 0,7813Hz data rate (when in hybrid mode)
   // [2]: lnoise=1 for low noise mode
   // [1]: f_read=0 for normal 16 bit reads
   // [0]: active=1 to take the part out of standby and enablesampling
   databyte = 0x3D;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   return 1;
}

// Used to configure the FXOS8700CQ
int1 configure_FXOS8700CQ() {
   int8 databyte = 0x00;  // Byte used for setup
   // write 0000 0000 = 0x00 to accelerometer control register 1 to place FXOS8700CQ into standby
   // [7-1] = 0000 000
   // [0]: active=0
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   //m_raw = 1
   databyte = 0x80;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG3, databyte) != 1) {
      return 0;
   }
   // write 0011 1101 = 0x3D to accelerometer control register 1
   // [7-6]: aslp_rate=00
   // [5-3]: dr=111 for 0,7813Hz data rate (when in hybrid mode)
   // [2]: lnoise=1 for low noise mode
   // [1]: f_read=0 for normal 16 bit reads
   // [0]: active=1 to take the part out of standby and enablesampling
   databyte = 0x3D;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   return 1;
}

//////////////////////////////////
///// Allround functions /////////
//////////////////////////////////

// Arctangens function which returns in the range of 0 to 2pi
float atan22(float y,float x) {
   float v;
   if (x>0) {
      v=atan(y/x);
   }

   if (y>=0 && x<0) {
    v=pi+atan(y/x);
   }

   if (y<0 && x<0) {
    v=-pi+atan(y/x);
   }

   if (y>0 && x==0) {
    v=pi/2;
   }

   if (y<0 && x==0) {
    v=-pi/2;
   }

   if (v<0) {
    v=v+2*pi;
   }
   
   return v;
}




//////////////////////////////////
//// Hardware close functions ////
//////////////////////////////////

// I2C write function that includes both chip selection, register selection
// and data entry
int1 s_i2c_write(int8 slave_address, int8 slave_register, int8 data) {
   i2c_start();
   if(i2c_write(slave_address)) {    // Select peripheral
      i2c_stop();
      return 0;  // Returns if no ACK
   }
   if(i2c_write(slave_register)) {  // Select internal register
      i2c_stop();
      return 0;
   }
   if(i2c_write(data)) {   // Puts data into selected internal register
      i2c_stop();
      return 0;
   }
   i2c_stop();
   return 1;
}

// I2C read function that reads a peripherals register with option of burst read
int1 s_i2c_read(int8 slave_address, int8 slave_register, int8 *data, int8 read_length) {
   int8 i;
   i2c_start();
   if(i2c_write(slave_address)) {    // Select peripheral
      i2c_stop();
      return 0;  // Returns if no ACK
   }
   if(i2c_write(slave_register)) {  // Select internal register
      i2c_stop();
      return 0;
   }
   i2c_start();   // For a read, it needs another start condition
   if(i2c_write(slave_address + 1)) {   // Select chip again with condition of a read
      i2c_stop();
      return 0;
   }
   if(read_length > 1) {   // Enters here if a burst read is needed
      for(i = 1; i < read_length; i++) {
         *data = i2c_read(1);
         data++;
      }
   }
   *data = i2c_read(0);   // Last read with a NOT ACK
   i2c_stop();
   return 1;
}


// Used for test
void test() {
   CLRScreen();
   int1 check;
   SRAWDATA acceleration;
   SRAWDATA magneticFlux;
   PutCursor(1,0);
   
   if (init_FXOS8700CQ()) {
      printf("Initializing FXOS8700C was succesful");
      check = 1;
   }
   else {
      printf("Initializing FXOS8700C has failed");
   }
   
   if (check) {
      if (readAccelMagnData(&acceleration, &magneticFlux)) {
         PutCursor(1,2);
         printf("Accelerometer x-component: %3f", acceleration.x*0.000488);
         PutCursor(1,3);
         printf("Accelerometer y-component: %3f", acceleration.y*0.000488);
         PutCursor(1,4);
         printf("Accelerometer z-component: %3f", acceleration.z*0.000488);
      }
   }
}

// Finds addresses on the I2C bus
void test2() {
   int8 i;
   for(i=0; i<112; i++) {
      i2c_start();
      if( ! i2c_write(i<<1) ) printf("%X",i<<1);
      i2c_stop();
   }
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
