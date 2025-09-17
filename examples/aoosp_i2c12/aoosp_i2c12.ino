// aoosp_i2c12.ino - demo illustrating the SAID I2C master in 12 bit mode
/*****************************************************************************
 * Copyright 2024,2025 by ams OSRAM AG                                       *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************/
#include <aospi.h>
#include <aoosp.h>


/*
DESCRIPTION
This demo shows how to read and write to an I2C devices using 12 bits mode.
The demo assumes an empty 8k byte EEPROM (i.e. filled with FFs) is connected 
to the I2C port of the OSP32. The demo resets and initializes the OSP chain, 
configures the SAID on the OSP32 board for 12 bits mode, then writes five 
bytes to location 0x12E of the EEPROM. After that a memory dump is made. 
To clean up, the five bytes are reset to 0xFF again.
Note-1 the used M24C64 is a 64kbit or 8kbyte EEPROM. This size requires 
log2(8k)=13 bits addresses. The SAID support 12 bits, so half of the EEPROM 
memory is not accessible by SAID. An M24C32 with 4kbyte would probably be more 
economical.
Note-2 at the end of this file there is a series of commands for 12 bit I2C
to use instead of this c-coded example.

HARDWARE
The demo runs on the OSP32 board with a terminator in OUT.
An M24C64 is connected to the I2C port. Its address pins are grounded to give 
it I2C address 0x50. Its WriteControl pin is also grounded to allow writes. 
See photos overview.jpg, detail.jpg, breakout.jpg and schematics.jpg for wiring.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
During the demo, there is no visible behavior; all is in the Serial output.
A capture of the logica analyzer is saved, see capture.sal or see the two 
screenshots write-0x12E.png and read-0x12E.png.

OUTPUT
Welcome to aoosp_i2c12.ino
version: result 0.4.6 spi 1.0.0 osp 0.8.0
spi: init(MCU-B)
osp: init

SETUP12
  mode 12

WRITE
  written 12E: 15 25 35 45 55

READ
  000: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  020: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  040: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  060: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  080: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  0A0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  0C0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  0E0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  100: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  120: FF FF FF FF FF FF FF FF FF FF FF FF FF FF 15 25   35 45 55 FF FF FF FF FF FF FF FF FF FF FF FF FF  
  140: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  160: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  180: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  1A0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  1C0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  
  1E0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  

WRITE
  written 12E: FF FF FF FF FF
*/


// Lazy way of error handling
#define PRINT_ERROR() do { if( result!=aoresult_ok ) { Serial.printf("ERROR %s\n", aoresult_to_str(result) ); } } while(0)


// OSP node address of the SAID to use for I2C
#define ADDR    0x001 // OSP32 has SAID1 with I2C on addr 001
// I2C device address
#define DADDR7   0x50 // The I2C header of OSP32 shall have an M24C64 EEPROM


// Setup OSP chain for 12 bits I2C
void i2c_setup12() {
  Serial.printf("\nSETUP12\n");
  aoresult_t result;

  // Reset all OSP nodes and power I2C bus
  result= aoosp_exec_resetinit(); PRINT_ERROR();
  result= aoosp_exec_i2cpower(ADDR); PRINT_ERROR();

  // Configure I2C for 12 bits mode
  result= aoosp_send_seti2ccfg(ADDR, AOOSP_I2CCFG_FLAGS_12BIT, AOOSP_I2CCFG_SPEED_DEFAULT ); PRINT_ERROR();
  uint8_t flags,speed;

  // Check I2C mode
  result= aoosp_send_readi2ccfg(ADDR, &flags, &speed ); PRINT_ERROR();
  Serial.printf("  mode %d\n", flags&AOOSP_I2CCFG_FLAGS_12BIT ? 12 : 8);
}


// Writes some bytes to eeprom
void i2c_write( bool clr ) {
  Serial.printf("\nWRITE\n");
  aoresult_t result;

  int raddr=0x12E;
  uint8_t buf[]= { 0x15, 0x25, 0x35, 0x45, 0x55 }; // OSP restriction: can only write 1, 3 or 5 byte
  if( clr ) for(int i=0; i<sizeof buf; i++ ) buf[i]=0xFF; // write back 0xFF if "clr" is requested
  result= aoosp_exec_i2cwrite12(ADDR, DADDR7, raddr, buf, sizeof buf); PRINT_ERROR();
  Serial.printf("  written %03X:",raddr); for(int i=0; i<sizeof buf;i++ ) Serial.printf(" %02X",buf[i]); Serial.printf("\n");
  delay(4); // Must wait for the write to complete, before sending next I2C transaction
}


// Prints (part of) the eeprom contents
void i2c_read() {
  Serial.printf("\nREAD\n");
  #define EEPROMS_SIZE 512 // Total number of bytes to read from eeprom
  #define READCHUNK      8 // Number of bytes to read per telegram (OSP supports max of 8)
  #define PRINTCHUNK    32 // Number of bytes to print per line
  uint8_t eeprom[EEPROMS_SIZE];
  aoresult_t result;

  // Read EEPROMS_SIZE bytes from EEPROM into eeprom[]
  for( int i=0; i<EEPROMS_SIZE; i+=READCHUNK ) {
    result= aoosp_exec_i2cread12(ADDR, DADDR7, i, &eeprom[i], READCHUNK); PRINT_ERROR();
  }

  // Print
  for( int a0=0; a0<EEPROMS_SIZE; a0+=PRINTCHUNK ) {
    Serial.printf("  %03X:",a0);
    for( int a1=a0; a1<a0+PRINTCHUNK; a1++ ) {
      Serial.printf(" %02X",eeprom[a1]);
      if( a1%16==15 ) Serial.printf("  ");
    }
    Serial.printf("\n");
  }
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_i2c12.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  i2c_setup12();
  i2c_write(false);
  i2c_read();
  i2c_write(true);
}


void loop() {
  delay(1000);
}


/*
>> osp resetinit
resetinit: bidir 001 (ok)

>> osp send 001 seti2ccfg 4C
tx A0 04 D7 4C 6F
rx none ok
>> osp send 001 readi2ccfg
tx A0 04 56 5D
rx A0 04 D6 4C 86 (49 us) ok
            ^
            
>> osp send 001 i2cwrite A0 00 00  C0 C1 C2 C3 C4
tx A0 07 99 A0 00 00 C0 C1 C2 C3 C4 CA
rx none ok
>> osp send 001 i2cwrite A0 00 05  C5 C6 C7 C8 C9
tx A0 07 99 A0 00 05 C5 C6 C7 C8 C9 BF
rx none ok

>> osp send 001 i2cread A0 00 08
tx A0 05 98 A0 00 08 4D
rx none ok
>> osp send 001 readlast
tx A0 04 1E EC
rx A0 07 9E C0 C1 C2 C3 C4 C5 C6 C7 18 (65 us) ok
            ^^^^^^^^^^^^^^^^^^^^^^^
            
>>  osp send 001 i2cwrite A0 01 00  D0 D1 D2 D3 D4
tx A0 07 99 A0 01 00 D0 D1 D2 D3 D4 FA
rx none ok
>> osp send 001 i2cwrite A0 01 05  D5 D6 D7 D8 D9
tx A0 07 99 A0 01 05 D5 D6 D7 D8 D9 8F
rx none ok

>> osp send 001 i2cread A0 10 28
tx A0 05 98 A0 10 28 1B
rx none ok
>> osp send 001 readlast
tx A0 04 1E EC
rx A0 07 9E D2 D3 D4 D5 D6 D7 D8 D9 71 (68 us) ok
            ^^^^^^^^^^^^^^^^^^^^^^^
*/

