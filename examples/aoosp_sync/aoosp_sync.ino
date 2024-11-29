// aoosp_sync.ino - demonstrates how one sync telegram activates all LEDs
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
This demo shows how to use the SYNC feature; a feature that switches on all 
LEDs at the same time. We first enable (channels) of nodes for SYNC
(AOOSP_CURCHN_FLAGS_SYNCEN). Next, set those channels to some PWM value.
Finally we issue a SYNC by broadcasting the SYNC telegram. This makes
all (channels) of all nodes that are configured for SYNC to activate their
PWM settings.

Instead of sending a SYNC telegram, there is also the option to use the
SYNC pin of the SAID (pin B1). To demonstrate that, define USE_HARDWARE_SYNC
and on the OSP32 board have jumper J8 connect SAID2.B2 to the ESP32 GPIO9
(instead of LED L2.1_B). Only SAID2 is wired for hardware sync on OSP32.
For this OTP must be written, so OTP password must be known and installed.

HARDWARE-1
The demo should run on the OSP32 board.
Have a cable from the OUT connector to the IN connector. 
**The jumper S2-B1 should connect to L2.1-B (default)**
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR-1
The serial monitor shows that SAID-1 channel-0 (L1.0) is set to green, 
but it does not yet light up on the board. Next, the Serial monitor shows 
that SAID-2 channel-0 is set to blue, but also this one does not light up.
When the serial monitor shows "SYNC via telegram" both light up in sync.

OUTPUT-1
Welcome to aoosp_sync.ino
version: result 0.1.10 spi 0.2.8 osp 0.2.2
spi: init
osp: init
SAID-1 channel-0 to green
SAID-2 channel-0 to blue
SYNC via telegram


HARDWARE-2
**The jumper S2-B1 should connect to 9/SYNC**

BEHAVIOR-2
The serial monitor shows that SAID-1 channel-0 (L1.0) is set to green, 
but it does not yet light up on the board. Next, the Serial monitor shows 
that SAID-2 channel-0 is set to blue, but also this one does not light up.
When the serial monitor shows "SYNC SAID-2 via pin" SAID-2 lights up blue.
Note that SAID-1 does not have its SYNC pin connected.

OUTPUT-2
Welcome to aoosp_sync.ino
version: result 0.4.1 spi 0.5.1 osp 0.4.1
spi: init
osp: init
SAID-1 channel-0 to green
SAID-2 channel-0 to blue
SYNC SAID-2 via pin
*/


// Set to 0 to use software sync (telegram) or 1 for hardware sync (gpio signal; SAID-2 only)
// Note: for hardware sync, also put jumper J8 on OSP32 to connect SAID2.1B to GPIO9
#define USE_HARDWARE_SYNC 0


// Print result if it is not ok
#define CHECK_RESULT(msg) do { if( result!=aoresult_ok ) Serial.printf("ERROR %d in %s (%s)\n", result, msg, aoresult_to_str(result) ); } while( 0 )


void demo_sync() {
  // Set up OSP chain
  aoresult_t result;
  uint16_t   last;
  int        loop;
  result= aoosp_exec_resetinit(&last,&loop); CHECK_RESULT("resetinit"); // restart entire chain
  if( last!=2 ) Serial.printf("WARNING unexpected OSP chain configuration (expected 2 nodes)\n");
  result= aoosp_send_clrerror(0x000); CHECK_RESULT("clrerror(000)"); // SAIDs have the V flag after boot, clear those
  result= aoosp_send_goactive(0x000); CHECK_RESULT("goactive(000)"); // Switch all nodes (broadcast) to active (allowing to switch on LEDs).

  #if USE_HARDWARE_SYNC  
    // Configure for hardware sync: ESP
    pinMode(9, OUTPUT);   // On OSP32 pin GPIO9/FREE/SYNC
    digitalWrite(9,HIGH); // Default line state for SYNC
    // Configure for hardware sync: (only) SAID2
    aoosp_exec_syncpinenable_set(0x002,1);
    // Note: also put jumper J8 on OSP32 to connect SAID2.1B to GPIO9
    // aoosp_exec_otpdump(0x002,AOOSP_OTPDUMP_CUSTOMER_ALL);
  #else
    // No init for software sync
  #endif

  // Configure SAID-1 channel-0 to wait for SYNC
  result= aoosp_send_setcurchn (0x001, 0, AOOSP_CURCHN_FLAGS_SYNCEN, AOOSP_CURCHN_CUR_DEFAULT, AOOSP_CURCHN_CUR_DEFAULT, AOOSP_CURCHN_CUR_DEFAULT ); CHECK_RESULT("aoosp_send_setcurchn(001,0)"); 
  // Configure SAID-2 channel-0 to wait for SYNC
  result= aoosp_send_setcurchn (0x002, 0, AOOSP_CURCHN_FLAGS_SYNCEN, AOOSP_CURCHN_CUR_DEFAULT, AOOSP_CURCHN_CUR_DEFAULT, AOOSP_CURCHN_CUR_DEFAULT ); CHECK_RESULT("aoosp_send_setcurchn(002,0)"); 

  // Configure SAID-1 channel-0 to go green
  Serial.printf("SAID-1 channel-0 to green\n");
  result= aoosp_send_setpwmchn(0x001, 0/*chn*/, 0x0000/*red*/, 0x7FFF/*green*/, 0x0000/*blue*/); CHECK_RESULT("setpwmchn(1,grn)");
  delay(2000);

  // Configure SAID-2 channel-0 to go blue
  Serial.printf("SAID-2 channel-0 to blue\n");
  result= aoosp_send_setpwmchn(0x002, 0/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x7FFF/*blue*/); CHECK_RESULT("setpwmchn(1,blu)");
  delay(2000);

  // Broadcast sync to activate the PWM settings
  #if USE_HARDWARE_SYNC  
    Serial.printf("SYNC SAID-2 via pin\n");
    digitalWrite(9,LOW); 
    delayMicroseconds(2); // SYNC pin must be pulsed for at least 3 clock cycles of the device to give a sync
    digitalWrite(9,HIGH); 
  #else
    Serial.printf("SYNC via telegram\n");
    result= aoosp_send_sync(0x000); CHECK_RESULT("sync(000)");
  #endif
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_sync.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  demo_sync();
}


void loop() {
  delay(5000);
}
