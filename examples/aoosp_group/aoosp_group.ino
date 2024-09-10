// aoosp_group.ino - two SAIDs in a group
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
This demo groups two SAIDs, then sends PWM commands to that group.
One command to switch the group to red, one command to switch the 
group to green. Then repeats.

HARDWARE
The demo runs on the OSP32 board. Have a cable from the OUT connector to 
the IN connector so that both SAIDs are in the chain.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
The first RGB (L1.0) of SAID OUT and the first RGB of the next SAID
blink: first both red then both green then repeats.

OUTPUT
Welcome to aoosp_group.ino
version: result 0.4.1 spi 0.5.1 osp 0.4.1

spi: init
osp: init

reset(0) ok
initloop(1) ok last 009
clrerror(0) ok
goactive(0) ok
setmult(1,grp5) ok
setmult(2,grp5) ok

setpwmchn(grp5,0,red) ok
setpwmchn(grp5,0,grn) ok

setpwmchn(grp5,0,red) ok
setpwmchn(grp5,0,grn) ok
*/


void group_init() {
  aoresult_t result;
  uint16_t   last;
  uint8_t    temp;
  uint8_t    stat;

  // Note: aoosp_exec_resetinit() implements reset and init with autodetect

  // Reset all nodes (broadcast) in the chain (all "off"; they also lose their address).
  result= aoosp_send_reset(0x000); 
  delayMicroseconds(150);
  Serial.printf("reset(0) %s\n", aoresult_to_str(result) );

  // Assigns an address to each node (starting from 1, serialcast, direction Loop).
  aospi_dirmux_set_loop();
  result= aoosp_send_initloop(0x001, &last, &temp, &stat);
  Serial.printf("initloop(1) %s last %03X\n", aoresult_to_str(result),last );

  // Clear the error flags of all node (broadcast).
  // SAIDs have the V flag (over-voltage) after reset, preventing them from going active.
  result= aoosp_send_clrerror(0x000); 
  Serial.printf("clrerror(0) %s\n", aoresult_to_str(result) );

  // Switch the state of all nodes (broadcast) to active (allowing to switch on LEDs).
  result= aoosp_send_goactive(0x000);
  Serial.printf("goactive(0) %s\n", aoresult_to_str(result) );

  // We put two SAIDs in one group, we picked group 5.
  // Note that the group register (MULT) is a bit mask, 
  // so one SAID could be in multiple groups.

  // Put SAID with address 1 in group 5.
  result= aoosp_send_setmult(0x001,1<<5);
  Serial.printf("setmult(1,grp5) %s\n", aoresult_to_str(result) );

  // Put SAID with address 2 in group 5.
  result= aoosp_send_setmult(0x002,1<<5);
  Serial.printf("setmult(2,grp5) %s\n", aoresult_to_str(result) );
}


void group_red() {
  // Set channel 0 red of group 5 to max
  aoresult_t result= aoosp_send_setpwmchn(AOOSP_ADDR_GROUP(5), 0/*chn*/, 0x7FFF/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  Serial.printf("setpwmchn(grp5,0,red) %s\n", aoresult_to_str(result) );
}


void group_green() {
  // Set channel 0 green of group 5 to max
    aoresult_t result= aoosp_send_setpwmchn(AOOSP_ADDR_GROUP(5), 0/*chn*/, 0x0000/*red*/, 0x7FFF/*green*/, 0x0000/*blue*/);
  Serial.printf("setpwmchn(grp5,0,grn) %s\n", aoresult_to_str(result) );
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_group.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );
  Serial.printf("\n" );

  aospi_init();
  aoosp_init();
  Serial.printf("\n" );

  group_init();
}


void loop() {
  Serial.printf("\n" );
  group_red();
  delay(500);
  group_green();
  delay(500);
}
