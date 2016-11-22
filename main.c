#include <main.h>

// Initialize the I2C
#use I2C (master, sda = PIN_C4, scl = PIN_C3)

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR 0x1E // with pins SA0=0, SA1=0

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS 0x00
#define FXOS8700CQ_WHOAMI 0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1 0x2A
#define FXOS8700CQ_M_CTRL_REG1 0x5B
#define FXOS8700CQ_M_CTRL_REG2 0x5C
#define FXOS8700CQ_WHOAMI_VAL 0xC7

// number of bytes to be read from the FXOS8700CQ
#define FXOS8700CQ_READ_LEN 13 // status plus 6 channels = 13 bytes

// Struct used for raw data output of FXOS8700CQ
typedef struct {
   int16 x;
   int16 y;
   int16 z;
} SRAWDATA;


/////////////////////
///// Functions /////
/////////////////////

// Used for initializing the
int1 init_FXOS8700CQ();

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


void main()
{
   // Oscillator has to be set up
   setup_oscillator(OSC_8MHZ, 14);
   // SDA(PIN_C4) and SCL(PIN_C3) have to be open collector
   output_float(PIN_C4);
   output_float(PIN_C3);
   
   test2();
   
   while(TRUE)
   {
      //TODO: User Code
   }

}




// read status and the three channels of accelerometer and
// magnetometer data from
// FXOS8700CQ (13 bytes)
int1 readAccelMagnData(SRAWDATA *pAccelData, SRAWDATA *pMagnData) {
   int8 Buffer[13]; // read buffer
   // read FXOS8700CQ_READ_LEN=13 bytes (status byte and the six channels of data)
   if(s_i2c_read(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_STATUS, &Buffer, 13)) {
      // copy the 14 bit accelerometer byte data into 16 bit words
      pAccelData->x = (int16)(((Buffer[1] << 8) | Buffer[2]))>> 2;
      pAccelData->y = (int16)(((Buffer[3] << 8) | Buffer[4]))>> 2;
      pAccelData->z = (int16)(((Buffer[5] << 8) | Buffer[6]))>> 2;
      // copy the magnetometer byte data into 16 bit words
      pMagnData->x = (Buffer[7] << 8) | Buffer[8];
      pMagnData->y = (Buffer[9] << 8) | Buffer[10];
      pMagnData->z = (Buffer[11] << 8) | Buffer[12];
   }
   else {
      return 0; // Returns if read not succesful
   }
   return 1;  // Returns if read succesful
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
   // write 0000 0001= 0x01 to XYZ_DATA_CFG register
   // [7]: reserved
   // [6]: reserved
   // [5]: reserved
   // [4]: hpf_out=0
   // [3]: reserved
   // [2]: reserved
   // [1-0]: fs=01 for accelerometer range of +/-4g range with 0.488mg/LSB
   databyte = 0x01;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_XYZ_DATA_CFG, databyte) != 1) {
      return 0;
   }
   // write 0000 1101 = 0x0D to accelerometer control register 1
   // [7-6]: aslp_rate=00
   // [5-3]: dr=001 for 200Hz data rate (when in hybrid mode)
   // [2]: lnoise=1 for low noise mode
   // [1]: f_read=0 for normal 16 bit reads
   // [0]: active=1 to take the part out of standby and enablesampling
   databyte = 0x0D;
   if(s_i2c_write(FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, databyte) != 1) {
      return 0;
   }
   return 1;
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
   int1 check;
   SRAWDATA acceleration;
   SRAWDATA magneticFlux;
   
   if (init_FXOS8700CQ()) {
      printf("Initializing FXOS8700C was succesful\n");
      check = 1;
   }
   else {
      printf("Initializing FXOS8700C has failed\n");
   }
   // 
   if (check) {
      if (readAccelMagnData(&acceleration, &magneticFlux)) {
         printf("Accelerometer x-component: %w16.0\n", acceleration.x);
         printf("Accelerometer y-component: %w16.0\n", acceleration.y);
         printf("Accelerometer z-component: %w16.0\n", acceleration.z);
      }
   }
}

// Finds addresses on the I2C bus
void test2() {
   int8 i;
   for(i=0; i<112; i++) {
      i2c_start();
      if( ! i2c_write(i<<1) ) printf("%X\n",i<<1);
      i2c_stop();
   }
}

