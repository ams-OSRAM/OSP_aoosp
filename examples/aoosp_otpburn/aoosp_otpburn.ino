// aoosp_otpburn.ino - demo how to burn the OTP of a SAID
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
#include <aoui32.h>


/*
DESCRIPTION
This demo burns a bit in the OTP (one time programmable memory) of a SAID.
The demo aoosp_otp.ino only writes in the non-persistent OTP mirror, this
demo actually updates the OTP persistently.
[warning 1] For OTP burning, the SAID password is needed, or a message is issued
  WARNING: ask ams-OSRAM for TESTPW and see aoosp_said_testpw_get() for how to set it
[warning 2] The OTP is a persistent memory. By default all bits are 0; the
transition to 1 can only be made once for every bit. In other words, OTP 
writes can not be undone (it is in the name: One Time Programmable memory). 
Some writes can change the behavior of the SAID (eg writes to CH_CLUSTERING 
at bits 0D.7, 0D.6 and OD.5), and writes to addresses below 0D could make 
the SAID unusable.
[warning 3] OTP burning is done with a lower voltage: a lab power supply is 
needed for the experiment in this sketch.

HARDWARE
This demo runs on the OSP32 board. Connect a SAIDlooker board in BiDir mode
(wire OSP32.OUT to SAIDlooker.IN and SAIDlooker.OUT to an OSP terminator).
Recall: this demo writes an OTP 0 bit to a 1 permanently, so the SAIDlooker
board will get some permanent 1s in its OTP.
In Arduino select board "ESP32S3 Dev Module", and flash this sketch.
There is no harm in running the sketch, because for the actual burn step 
the user must still press the A-button on the OSP32 board. Even if that is 
pressed, the burn will not succeed because of the too high supply voltage.
After this sketch is flashed, the USB cable between the OP32 board and the
PC should be disconnected. The OSP32 board shall be 5V powered via J3C only, 
using a lab power supply. During the burn, the OLED will instruct to
reduce the supply from 5V to 4V.

BEHAVIOR
- Make sure the SAID password is enabled (see aoosp.cpp line ~30).
- Update the constants ADDR, OTPADDR, OTPDATA.
- Compile and upload this script as usual (it will not do OTP burn on its own).
- Now unplug all USB cables, and supply with a lab supply 5V via the OSP32 
  pin header in the lower right corner (J3C).
- Reset the ESP32, the OLED should say
    SAID[XXX].OTP[YY]=ZZ Press A to change the P2RAM
  where XX is ADDR, YY is OTPADDR and ZZ the old value (not yet OTPDATA).
- Press the A button on the OSP32 board, the OLED should say
    Reduce 5.0 V to 4.0 V and press A to burn
  (Note 4.0V is on the high side for OTP burning, but this is needed to have
  headroom for the LDO on the ESP32 board to generate 3V3).
- Without interrupting power, reduce the voltage of the power supply 
  from 5V to 4V. Then press A.
- The OLED should say
    Increase 4.0 V to 5.0 V and press A to test
- Without interrupting power, increase the voltage of the power supply 
  from 4.0 V to 5.0 V. Then press A.
- Finally, the OLED should say
    OTP[XX] was AA burn BB now CC - sketch end
  where XX is OTPADDR and AA was the old value in the OTP,
  BB the value written (ie OTPDATA), and CC new value (it should OTPDATA
  or rather AA | OTPDATA )

OUTPUT (run on USB - so burning does not happen because voltage is too high)
Welcome to aoosp_otpburn.ino
version: result 0.4.5 spi 0.5.7 osp 0.5.0 ui32 0.3.9
spi: init
osp: init
ui32: init

Burn OTP of 003 at 1D with 01

old otp: 0x0D: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
new otp: 0x0D: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00
inv otp: 0x0D: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FE 00 00
now otp: 0x0D: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
OTP[1D] was 00 burn 01 now 00 - sketch end
*/


// OTP burn addresses
#define ADDR    0x003 // address of the SAID whose OTP gets burned
#define OTPADDR 0x1D  // address of OTP byte we want to change
#define OTPDATA 0x01  // new value to be written (this is ORed with the current value)


void wait_for_button_A( const char * msg ) {
  aoui32_oled_msg(msg);
  while( aoui32_but_isdown( AOUI32_BUT_A) ) { aoui32_but_scan(); }
  while( aoui32_but_isup  ( AOUI32_BUT_A) ) { aoui32_but_scan(); }
}

#define CHECKRESULT(tele) do { if( result!=aoresult_ok ) { Serial.printf("ERROR in " tele ": %s\n", aoresult_to_str(result) ); return; } } while(0)


void otp_burn() {
  aoresult_t result;
  uint16_t   last;
  int        loop;
  uint8_t    temp;
  uint8_t    stat;
  uint8_t    data1,data2,data3;
  char       msg[100];

  Serial.printf("\nBurn OTP of %03x at %02X with %02X\n\n",ADDR,OTPADDR, OTPDATA);

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_exec_resetinit(&last,&loop); CHECKRESULT("aoosp_exec_resetinit");

  // PREPARE IMAGE (in P2RAM)
  // OTP hexdump for human checking
  Serial.printf("old ");
  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX ); CHECKRESULT("aoosp_exec_otpdump");
  // Wait for human to read
  result= aoosp_send_readotp(ADDR,OTPADDR,&data1,1); CHECKRESULT("aoosp_send_readotp");
  sprintf(msg,"SAID[%03X].OTP[%02X]=%02X    Press A to change the P2RAM", ADDR, OTPADDR, data1 );
  wait_for_button_A( msg );
  // Write to P2RAM - update of the mirror (in ram 1 one can be changed to 0, but not in the OTP)
  result= aoosp_exec_setotp(ADDR, OTPADDR, OTPDATA); CHECKRESULT("aoosp_exec_setotp");
  // OTP hexdump for human checking
  Serial.printf("new ");
  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX ); CHECKRESULT("aoosp_exec_otpdump");

  // BURN OTP (P2RAM to fuses)
  // Authenticate to enable burning
  result= aoosp_send_settestpw_sr(ADDR,aoosp_said_testpw_get(),&temp,&stat); CHECKRESULT("aoosp_send_settestpw_sr");
  // Wait for operator to reduce supply from 5V to 4V
  wait_for_button_A( "Reduce 5.0 V to 4.0 V and press A to burn" );
  // Select customer area
  result= aoosp_send_cust(ADDR); CHECKRESULT("aoosp_send_cust");
  // Burn the P2RAM to OTP
  result= aoosp_send_burn(ADDR); CHECKRESULT("aoosp_send_burn");
  // Wait
  delay(5); 
  data2 = data1 | OTPDATA; // expected value in the OTP
  // Switch OTP back to idle mode
  result= aoosp_send_idle(ADDR); CHECKRESULT("aoosp_send_idle");
  // De-authenticate
  result= aoosp_send_settestpw_sr(ADDR,0,&temp,&stat); CHECKRESULT("aoosp_send_settestpw_sr");

  // INVALIDATE P2RAM (for verification)
  // Wait for operator to increase supply back from 4V to 5V
  wait_for_button_A( "Increase 4.0 V to 5.0 V and press A to test" );
  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_exec_resetinit(&last,&loop); CHECKRESULT("aoosp_exec_resetinit");
  // Invalidate the OTP mirror at the write location
  result= aoosp_exec_setotp(ADDR, OTPADDR, ~OTPDATA,0x00); CHECKRESULT("aoosp_exec_setotp");
  // OTP hexdump for human checking
  Serial.printf("inv ");
  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX ); CHECKRESULT("aoosp_exec_setotp");

  // VERIFY OTP
  // Authenticate to enable reload
  result = aoosp_send_settestpw(ADDR,aoosp_said_testpw_get()); CHECKRESULT("aoosp_send_settestpw");
  // Select customer area
  result= aoosp_send_cust(ADDR); CHECKRESULT("aoosp_send_cust");
  // Set a different guard-band for the LOAD operation; enables thorough fuse burn check
  result= aoosp_send_gload(ADDR); CHECKRESULT("aoosp_send_gload");
  // Load the values from the OTP to the P2RAM
  result= aoosp_send_load(ADDR); CHECKRESULT("aoosp_send_load");
  // Switch OTP back to idle mode
  result= aoosp_send_idle(ADDR); CHECKRESULT("aoosp_send_idle");
  // De-authenticate
  result= aoosp_send_settestpw_sr(ADDR,0,&temp,&stat); CHECKRESULT("aoosp_send_settestpw_sr");
  // OTP hexdump for human checking
  Serial.printf("now ");
  result= aoosp_exec_otpdump(ADDR, AOOSP_OTPDUMP_CUSTOMER_HEX ); CHECKRESULT("aoosp_exec_otpdump");
  // Confirm to human
  result= aoosp_send_readotp(ADDR,OTPADDR,&data3,1); CHECKRESULT("aoosp_send_readotp");
  Serial.printf("OTP[%02X] was %02X burn %02X now %02X - sketch end", OTPADDR, data1, data2, data3 );
  sprintf(msg,"OTP[%02X] was %02X burn %02X now %02X - sketch end", OTPADDR, data1, data2, data3 );
  wait_for_button_A( msg );
}


void setup() {
  Serial.begin(115200); 
  Serial.printf("\n\nWelcome to aoosp_otpburn.ino\n");
  Serial.printf("version: result %s spi %s osp %s ui32 %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION, AOUI32_VERSION );

  aospi_init();
  aoosp_init();
  aoui32_init();

  otp_burn();
}


void loop() {
  delay(5000);
}
