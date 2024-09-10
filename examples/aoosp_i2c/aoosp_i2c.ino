// aoosp_i2c.ino - demo how use the I2C master in a SAID
/*****************************************************************************
 * Copyright 2024 by ams OSRAM AG                                            *
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
This demo first performs an I2C scan using the I2C bridge in a SAID.
Then it issues I2C read and write transactions to an EEPROM memory,
assumed to have I2C device address 0x50 connected to the first SAID.
Finally it polls the INT line and shows its status on SAID1.RGB0.

HARDWARE
The demo runs on the OSP32 board. Have a cable from the OUT connector to 
the IN connector so that both SAIDs are in the chain.
The SAID OUT on the OSP32 board is enabled for I2C. On the OSP32 board an I2C 
device (EEPROM) is attached to it. One could attach other I2C devices, like 
the supplied EEPROM stick, via the header labeled "I2C".
The first phase of this demo, scans for all connected I2C devices on the bus 
(it finds at least the EEPROM). The second phase writes/reads the connected 
EEPROM. The OSP32 board has a pushbutton connected to the INT line of the 
SAID with I2C bridge. In the third phase, pressing the push button instructs
the firmware to change the color of an LED.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
During the I2C scan or the EEPROM test, there is no visible behavior.
During the third phase the first RGB (L1.0) of SAID OUT is green
except while the INT button is depressed, then it is red.

OUTPUT
Welcome to aoosp_i2c.ino
version: result 0.4.1 spi 0.5.1 osp 0.4.1
spi: init
osp: init

resetinit last 002 loop

Bus scan for SAID 001
00:  00  01  02  03  04  05  06  07 
08:  08  09  0a  0b  0c  0d  0e  0f 
10:  10  11  12  13  14  15  16  17 
18:  18  19  1a  1b  1c  1d  1e  1f 
20:  20  21  22  23  24  25  26  27 
28:  28  29  2a  2b  2c  2d  2e  2f 
30:  30  31  32  33  34  35  36  37 
38:  38  39  3a  3b  3c  3d  3e  3f 
40:  40  41  42  43  44  45  46  47 
48:  48  49  4a  4b  4c  4d  4e  4f 
50:  50  51  52  53 [54] 55  56  57 
58:  58  59  5a  5b  5c  5d  5e  5f 
60:  60  61  62  63  64  65  66  67 
68:  68  69  6a  6b  6c  6d  6e  6f 
70:  70  71  72  73  74  75  76  77 
78:  78  79  7a  7b  7c  7d  7e  7f 
SAID 001 has 1 I2C devices (see square brackets)

resetinit last 002 loop

Read/write (locations 00..07 of EEPROM 54 on SAID 001)
eeprom FF FF FF FF FF FF FF FF (original)
eeprom FF FF BE EF FF FF FF FF (written)
eeprom FF FF FF FF FF FF FF FF (restored)

resetinit last 002 loop

Press INT button and check L1.0
*/


// OSP node address of the SAID to use for I2C
#define ADDR    0x001 // OSP32 has SAID1 with I2C on addr 001
// I2C device address
#define DADDR   0x54 // OSP32 has EEPROM with this address on SAID1


// Print a table of all I2C device addresses that acknowledge a read
void i2c_scan() {
  aoresult_t result;
  int        enable;
  uint16_t   last;
  int        loop;

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_exec_resetinit(&last,&loop); 
  if( result!=aoresult_ok ) { Serial.printf("resetinit %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("resetinit last %03X %s\n", last, loop?"loop":"bidir" );

  // In case the SAID does not have the I2C_BRIDGE_EN, we could try to override this in RAM
  //result= aoosp_exec_i2cenable_set(ADDR,1);
  //if( result!=aoresult_ok ) { Serial.printf("i2cenable %s\n", aoresult_to_str(result) ); return; }
  // Check for an I2C bridge
  result= aoosp_exec_i2cenable_get(ADDR,&enable);
  if( result!=aoresult_ok || !enable ) { Serial.printf("ERROR: there doesn't seem to be a SAID with I2C enabled at %03X (%s)\n",ADDR,aoresult_to_str(result)); return; }

  // Power the I2C bus
  result= aoosp_exec_i2cpower(ADDR);
  if( result!=aoresult_ok ) { Serial.printf("i2cpower %s\n", aoresult_to_str(result) ); return; }

  // Scan all devices
  Serial.printf("\nBus scan for SAID %03X\n",ADDR);
  int count= 0;
  int found= 0;
  for( uint8_t daddr7=0; daddr7<0x80; daddr7++ ) {
    if( daddr7 % 8 == 0) Serial.printf("%02x: ",daddr7);
    // Try to read at address 0 of device daddr7
    uint8_t buf[8];
    result = aoosp_exec_i2cread8(ADDR, daddr7, 0x00, buf, 1);
    int i2cfail=  result==aoresult_dev_i2cnack || result==aoresult_dev_i2ctimeout;
    if( result!=aoresult_ok && !i2cfail )  { Serial.printf("i2cread8 %s\n", aoresult_to_str(result) ); return; }
    if( i2cfail ) Serial.printf(" %02x ",daddr7); else Serial.printf("[%02x]",daddr7); // [] brackets indicate presence
    if( !i2cfail && daddr7==DADDR ) found++;
    if( !i2cfail ) count++;
    if( daddr7 % 8 == 7) Serial.printf("\n");
  }
  Serial.printf("SAID %03X has %d I2C devices (see square brackets)\n", ADDR, count);
  if( !found ) Serial.printf("WARNING: expect I2C device with address %02X, but did find any\n",DADDR);

  Serial.printf("\n");
}


// I2C reads and writes to an eeprom
void i2c_eeprom() {
  aoresult_t result;
  int        enable;
  uint16_t   last;
  int        loop;
  uint8_t    rbuf1[8];
  uint8_t    rbuf2[8];
  uint8_t    wbuf[2];

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_exec_resetinit(&last,&loop); 
  if( result!=aoresult_ok ) { Serial.printf("resetinit %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("resetinit last %03X %s\n", last, loop?"loop":"bidir" );

  // In case the SAID does not have the I2C_BRIDGE_EN, we could try to override this in RAM
//result= aoosp_exec_i2cenable_set(ADDR,1);
//if( result!=aoresult_ok ) { Serial.printf("i2cenable %s\n", aoresult_to_str(result) ); return; }
  // Check for an I2C bridge
  result= aoosp_exec_i2cenable_get(ADDR,&enable);
  if( result!=aoresult_ok || !enable ) { Serial.printf("ERROR: there doesn't seem to be a SAID with I2C enabled at %03X (%s)\n",ADDR,aoresult_to_str(result)); return; }

//uint8_t flags, rcur, gcur, bcur;
//result= aoosp_send_readcurchn(ADDR, 2, &flags, &rcur, &gcur, &bcur);
//if( result!=aoresult_ok ) { Serial.printf("readcurchn %s\n", aoresult_to_str(result) ); return; }
//Serial.printf("readcurchn: flags %02X rgb %02X %02X %02X\n", flags, rcur, gcur, bcur);

  // Power the I2C bus
  result= aoosp_exec_i2cpower(ADDR);
  if( result!=aoresult_ok ) { Serial.printf("i2cpower %s\n", aoresult_to_str(result) ); return; }

  Serial.printf("\nRead/write (locations 00..07 of EEPROM %02X on SAID %03X)\n",DADDR,ADDR);
  // Dump the first 8 bytes of the I2C EEPROM memory
  result= aoosp_exec_i2cread8(ADDR, DADDR, 0x00, rbuf1, 8); 
  if( result!=aoresult_ok ) { Serial.printf("i2cread8 %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("eeprom");
  for( int i=0; i<8; i++ ) Serial.printf(" %02X",rbuf1[i]);
  Serial.printf(" (original)\n");

  // Modify bytes 2 and 3 of the I2C EEPROM memory
  wbuf[0]= 0xBE; wbuf[1]=0xEF;
  result= aoosp_exec_i2cwrite8(ADDR, DADDR, 0x02, wbuf, 2); 
  if( result!=aoresult_ok ) { Serial.printf("i2cwrite8 %s\n", aoresult_to_str(result) ); return; }

  // Dump the first 8 bytes of the I2C EEPROM memory
  result= aoosp_exec_i2cread8(ADDR, DADDR, 0x00, rbuf2, 8); 
  if( result!=aoresult_ok ) { Serial.printf("i2cread8 %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("eeprom");
  for( int i=0; i<8; i++ ) Serial.printf(" %02X",rbuf2[i]);
  Serial.printf(" (written)\n");

  // Restore bytes 2 and 3 of the I2C EEPROM memory
  wbuf[0]= rbuf1[2]; wbuf[1]=rbuf1[3];
  result= aoosp_exec_i2cwrite8(ADDR, DADDR, 0x02, wbuf, 2); 
  if( result!=aoresult_ok ) { Serial.printf("i2cwrite8 %s\n", aoresult_to_str(result) ); return; }

  // Dump the first 8 bytes of the I2C EEPROM eepromory
  result= aoosp_exec_i2cread8(ADDR, DADDR, 0x00, rbuf2, 8); 
  if( result!=aoresult_ok ) { Serial.printf("i2cread8 %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("eeprom");
  for( int i=0; i<8; i++ ) Serial.printf(" %02X",rbuf2[i]);
  Serial.printf(" (restored)\n");

  Serial.printf("\n");
}


// Poll the INT pin
#define ADDR_LED 0x001
void i2c_int_setup() {
  aoresult_t result;
  int        enable;
  uint16_t   last;
  int        loop;

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_exec_resetinit(&last,&loop); 
  if( result!=aoresult_ok ) { Serial.printf("resetinit %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("resetinit last %03X %s\n", last, loop?"loop":"bidir" );

  // In case the SAID does not have the I2C_BRIDGE_EN, we could try to override this in RAM
//result= aoosp_exec_i2cenable_set(ADDR,1);
//if( result!=aoresult_ok ) { Serial.printf("i2cenable %s\n", aoresult_to_str(result) ); return; }
  // Check for an I2C bridge
  result= aoosp_exec_i2cenable_get(ADDR,&enable);
  if( result!=aoresult_ok || !enable ) { Serial.printf("ERROR: there doesn't seem to be a SAID with I2C enabled at %03X (%s)\n",ADDR,aoresult_to_str(result)); return; }

  // Power the I2C bus
  result= aoosp_exec_i2cpower(ADDR);
  if( result!=aoresult_ok ) { Serial.printf("i2cpower %s\n", aoresult_to_str(result) ); return; }

  // Prepare SAID at ADDR_LED for feedback (clear its errors and go active)
  result= aoosp_send_clrerror(ADDR_LED); 
  if( result!=aoresult_ok ) { Serial.printf("clrerror %s\n", aoresult_to_str(result) ); return; }
  result= aoosp_send_goactive(ADDR_LED);
  if( result!=aoresult_ok ) { Serial.printf("goactive %s\n", aoresult_to_str(result) ); return; }

  // Instruct user
  Serial.printf("\nPress INT button and check L1.0\n");
}


void i2c_int_loop() {
  aoresult_t result;

  uint8_t flags, speed;
  result= aoosp_send_readi2ccfg(ADDR, &flags, &speed);
  if( result!=aoresult_ok ) { Serial.printf("readi2ccfg %s\n", aoresult_to_str(result) ); return; }
  uint8_t intstate= flags & AOOSP_I2CCFG_FLAGS_INT;
  if( intstate ) {
    result= aoosp_send_setpwmchn(ADDR_LED, 0/*chn*/, 0x7FFF/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
    if( result!=aoresult_ok ) { Serial.printf("setpwmchn %s\n", aoresult_to_str(result) ); return; }
  } else {
    result= aoosp_send_setpwmchn(ADDR_LED, 0/*chn*/, 0x0000/*red*/, 0x7FFF/*green*/, 0x0000/*blue*/);
    if( result!=aoresult_ok ) { Serial.printf("setpwmchn %s\n", aoresult_to_str(result) ); return; }
  }

  delay(1);
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_i2c.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();
  //aoosp_loglevel_set(aoosp_loglevel_tele);
  Serial.printf("\n");

  i2c_scan();

  i2c_eeprom();

  i2c_int_setup();
}


void loop() {
  i2c_int_loop();
}
