// aoosp_min.ino - minimal series of OSP telegrams that light up an LED
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
This demo sends a minimal set of OSP telegrams that light up an LED
connected to a SAID channel. Once using BiDir, once using Loop.
Then repeats.

HARDWARE
The demo should run on the OSP32 board.
Have a cable from the OUT connector to the IN connector. 
This demo will run alternately in BiDir and Loop mode.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
The first RGB (L1.0) of SAID OUT blinks bright white and dim white.
First in BiDir mode (so direction mux led is green) then in loop (led is orange).
Then repeats.

OUTPUT
Welcome to aoosp_min.ino
version: result 0.1.10 spi 0.2.8 osp 0.2.2
spi: init
osp: init

reset(0x000) [tele A0 00 00 22]
reset ok
initbidir(0x001) [tele A0 04 02 A9] -> [resp A0 09 02 00 50 6D] last=0x02=2 temp=0x00=-86 stat=0x50=SLEEP:tV:clou (-126, SLEEP:oL:clou)
initbidir ok last 002
clrerror(0x000) [tele A0 00 01 0D]
clrerror ok
goactive(0x000) [tele A0 00 05 B1]
goactive ok
setpwmchn(0x001,0,0x7FFF,0x7FFF,0x7FFF) [tele A0 07 CF 00 FF 7F FF 7F FF 7F FF 74]
setpwmchn ok
setpwmchn(0x001,0,0x0FFF,0x0FFF,0x0FFF) [tele A0 07 CF 00 FF 0F FF 0F FF 0F FF 85]
setpwmchn ok

reset(0x000) [tele A0 00 00 22]
reset ok
initloop(0x001) [tele A0 04 03 86] -> [resp A0 09 03 00 50 63] last=0x02=2 temp=0x00=-86 stat=0x50=SLEEP:tV:clou (-126, SLEEP:oL:clou)
initloop ok last 002
clrerror(0x000) [tele A0 00 01 0D]
clrerror ok
goactive(0x000) [tele A0 00 05 B1]
goactive ok
setpwmchn(0x001,0,0x7FFF,0x7FFF,0x7FFF) [tele A0 07 CF 00 FF 7F FF 7F FF 7F FF 74]
setpwmchn ok
setpwmchn(0x001,0,0x0FFF,0x0FFF,0x0FFF) [tele A0 07 CF 00 FF 0F FF 0F FF 0F FF 85]
setpwmchn ok
*/


void send_min(int loop) {
  aoresult_t result;
  uint16_t   last;
  uint8_t    temp;
  uint8_t    stat;

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_send_reset(0x000); 
  delayMicroseconds(150);
  Serial.printf("reset %s\n", aoresult_to_str(result) );

  // Assigns an address to each node (starting from 1, serialcast).
  // Also configures all nodes for Loop or BiDir.
  if( loop ) {
    aospi_dirmux_set_loop();
    result= aoosp_send_initloop(0x001, &last, &temp, &stat);
    Serial.printf("initloop %s last %03X\n", aoresult_to_str(result),last );
  } else {
    aospi_dirmux_set_bidir();
    result= aoosp_send_initbidir(0x001, &last, &temp, &stat);
    Serial.printf("initbidir %s last %03X\n", aoresult_to_str(result), last );
  }
  // Note: aoosp_exec_resetinit() implements reset and init with autodetect

  // Clear the error flags of all nodes (broadcast).
  // SAIDs have the V flag (over-voltage) after reset, preventing them from going active.
  result= aoosp_send_clrerror(0x000); 
  Serial.printf("clrerror %s\n", aoresult_to_str(result) );

  // Switch the state of all nodes (broadcast) to active (allowing to switch on LEDs).
  result= aoosp_send_goactive(0x000);
  Serial.printf("goactive %s\n", aoresult_to_str(result) );

  // Set three PWM values of channel 0 of node 0x001 (unicast) to max (node 1 must be a SAID, otherwise use aoosp_send_setpwm)
  result= aoosp_send_setpwmchn(0x001, 0/*chn*/, 0x7FFF/*red*/, 0x7FFF/*green*/, 0x7FFF/*blue*/);
  Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );

  delay(500);

  result= aoosp_send_setpwmchn(0x001, 0/*chn*/, 0x0FFF/*red*/, 0x0FFF/*green*/, 0x0FFF/*blue*/);
  Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );

  delay(500);
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_min.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();
  aoosp_loglevel_set( aoosp_loglevel_tele );
  Serial.printf("\n");
}


void loop() {
  send_min(0);
  Serial.printf("\n");
  send_min(1);
  Serial.printf("\n");
}
