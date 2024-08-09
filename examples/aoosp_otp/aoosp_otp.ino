// aoosp_otp.ino - demo how to read and write OTP
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
This demo reads and writes from/to the OTP (one time programmable memory)
of a SAID.
Note that this accesses the OTP mirror in RAM, not the actual OTP ("ROM").
The mirror is persistent over RESET, but not over POR (power on reset).
The latter requires the CUST, BURN, IDLE steps, which are beyond the scope 
of this example.

HARDWARE
The demo should run on the OSP32 board.
Have a cable from the OUT connector to the IN connector. 
In Arduino select board "ESP32S3 Dev Module".

OUTPUT
Welcome to aoosp_otp.ino
version: result 0.1.10 spi 0.2.8 osp 0.2.2
spi: init
osp: init

DUMP of 001
resetinit last 002 loop

otp: 0x0D: 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
otp: CH_CLUSTERING     0D.7:5 0
otp: HAPTIC_DRIVER     0D.4   0
otp: SPI_MODE          0D.3   1
otp: SYNC_PIN_EN       0D.2   0
otp: STAR_NET_EN       0D.1   0
otp: I2C_BRIDGE_EN     0D.0   0
otp: *STAR_START       0E.7   0
otp: OTP_ADDR_EN       0E.3   0
otp: STAR_NET_OTP_ADDR 0E.2:0 0 (0x000)

READ/WRITE DEMO of 001
resetinit last 002 loop

otp: 0x0D: 09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
10 <- 5A
otp: 0x0D: 09 00 00 5A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
10 <- 00
otp: 0x0D: 09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
flip 0D.0
otp: 0x0D: 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
flip 0D.0
otp: 0x0D: 09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*/


// OTP of which node to dump
#define ADDR 0x001


void otp_dump() {
  aoresult_t result;
  uint16_t   last;
  int        loop;

  Serial.printf("\nDUMP of %03x\n",ADDR);

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_exec_resetinit(&last,&loop); 
  if( result!=aoresult_ok ) { Serial.printf("resetinit %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("resetinit last %03X %s\n", last, loop?"loop":"bidir" );
  Serial.printf("\n");


  // Dump, update 10, dump, restore 10, dump
  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_ALL );
  if( result!=aoresult_ok ) { Serial.printf("otpdump %s\n", aoresult_to_str(result) ); return; }
}


void otp_demo() {
  aoresult_t result;
  uint16_t   last;
  int        loop;
  int        enable;

  Serial.printf("\nREAD/WRITE DEMO of %03x\n",ADDR);

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_exec_resetinit(&last,&loop); 
  if( result!=aoresult_ok ) { Serial.printf("resetinit %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("resetinit last %03X %s\n", last, loop?"loop":"bidir" );
  Serial.printf("\n");


  // Dump, update 10, dump, restore 10, dump
  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX );
  if( result!=aoresult_ok ) { Serial.printf("otpdump %s\n", aoresult_to_str(result) ); return; }

  result= aoosp_exec_setotp(ADDR, 0x10, 0x5A);
  if( result!=aoresult_ok ) { Serial.printf("setotp %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("10 <- 5A\n");

  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX );
  if( result!=aoresult_ok ) { Serial.printf("otpdump %s\n", aoresult_to_str(result) ); return; }

  result= aoosp_exec_setotp(ADDR, 0x10, 0x00,0x00);
  if( result!=aoresult_ok ) { Serial.printf("setotp %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("10 <- 00\n");

  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX );
  if( result!=aoresult_ok ) { Serial.printf("otpdump %s\n", aoresult_to_str(result) ); return; }


  // Get I2C, flip, dump, flip
  result = aoosp_exec_i2cenable_get(ADDR, &enable);
  if( result!=aoresult_ok ) { Serial.printf("i2cenable_get %s\n", aoresult_to_str(result) ); return; }

  result = aoosp_exec_i2cenable_set(ADDR, !enable);
  if( result!=aoresult_ok ) { Serial.printf("i2cenable_get %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("flip 0D.0\n");

  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX );
  if( result!=aoresult_ok ) { Serial.printf("otpdump %s\n", aoresult_to_str(result) ); return; }

  result = aoosp_exec_i2cenable_set(ADDR, enable);
  if( result!=aoresult_ok ) { Serial.printf("i2cenable_get %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("flip 0D.0\n");

  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX );
  if( result!=aoresult_ok ) { Serial.printf("otpdump %s\n", aoresult_to_str(result) ); return; }
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_otp.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  otp_dump();
  otp_demo();
}


void loop() {
  delay(5000);
}
