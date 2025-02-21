// aoosp_ledst.ino - demonstrates LED (open/short) status (READLEDSTCHN)
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
This demo shows how how to obtain the LOS (LED Open Short) status via 
the telegram readledst[chn]. An LED driver typically sinks current for 
an LED that is connected to VCC (+5V). The LED has a forward voltage 
(of let's say of 2V), so the driver output pin measures at 5V - 2V or 3V. 
If the voltage is substantially higher (nearing 5V) the driver output is 
said to to be _short_ to VCC (there is no forward voltage, the driver 
directly connects to VCC). If the voltage is low (nearing 0V), the driver
is not connected to the LED (or connected to ground). It is said to be
_open_.

HARDWARE
The demo is made for OSP32 with its OUT connected to its IN.
In Arduino select board "ESP32S3 Dev Module".

At the back of the OSP32 board is a jumper. Its center pin S2_B1 is 
SAID IN (S2) its B1 (blue of channel 1). By default, the jumper connects 
S2_B1 to L2.1_B, that is RGB module L2.1 its B channel. There are
four options for the jumper and thus for runs of this sketch.
- run 1: have the jumper in its default position
- run 2: remove the jumper; SAID IN channel 1 blue is open
- run 3: use a jumper wire to connect the center pin to 5V
- run 4: use a jumper wire to connect the center pin to GND

BEHAVIOR
The sketch drives all LEDs with a PWM of FFF, so all five RGB modules on 
the OSP32 board turn white. However, in run 2, 3, and 4 SAID IN its B1 is 
disconnected, so the middle RGB module at the bottom does not have blue on,
so it turns yellow.
The important feedback of this demo is in the Serial out. We see os-os-os
for channel 0, then for channel 1 then for channel 2. We focus on channel 1,
and there focus on the last "os", it is for the blue LED. Legend:
  r0-g0-b0 r1-g1-b1 r2-g2-b2
  os-os-os os-os-os os-os-os
                 ^^ look here
Capital O or S means Open respectively Short detected.

OUTPUT (run 1 - B1 via jumper connected to RGB module)
Welcome to aoosp_ledst.ino
version: result 0.4.5 spi 0.5.7 osp 0.5.0
spi: init
osp: init
SAID(002) os-os-os os-os-os os-os-os

OUTPUT (run 2 - jumper removed, B1 open)
Welcome to aoosp_ledst.ino
version: result 0.4.5 spi 0.5.7 osp 0.5.0
spi: init
osp: init
SAID(002) os-os-os os-os-Os os-os-os

OUTPUT (run 3 - B1 wired to 5V)
Welcome to aoosp_ledst.ino
version: result 0.4.5 spi 0.5.7 osp 0.5.0
spi: init
osp: init
SAID(002) os-os-os os-os-oS os-os-os

OUTPUT (run 4 - B1 wired to GND)
Welcome to aoosp_ledst.ino
version: result 0.4.5 spi 0.5.7 osp 0.5.0
spi: init
osp: init
SAID(002) os-os-os os-os-Os os-os-os
*/


// Print result if it is not ok
#define CHECK_RESULT(msg) do { if( result!=aoresult_ok ) Serial.printf("ERROR %d in %s (%s)\n", result, msg, aoresult_to_str(result) ); } while( 0 )


// Prints for the SAID at addr the status of the R, G and B driver of all three channels
// "Open" means the driver output pin has a low voltage: either the output pin is dangling (not connected at all),  or the pin is grounded (or the LED is blown open).
// "Short" means the driver output pin has a high voltage: the output pin connected to VCC (or the LED is short circuited).
void print_said_ledst(uint16_t addr) {
  Serial.printf("SAID(%03X)",addr );
  for( uint8_t chn=0; chn<3; chn++ ) {
    uint8_t ledst;
    aoresult_t result= aoosp_send_readledstchn(addr, chn, &ledst ); CHECK_RESULT("readledstchn(addr,chn)"); 
    //Serial.printf(" rgb%d:%02x:%s", chn, ledst, aoosp_prt_ledst(ledst) );
    Serial.printf(" %s", aoosp_prt_ledst(ledst) ); 
  }
  Serial.printf( "\n" );
}


// Sets up the OSP chain and inspects the LOS (LED open/short status) of SAID at 002
void demo() {
  aoresult_t result;
  uint16_t   last;
  int        loop;

  // Set up OSP chain
  result= aoosp_exec_resetinit(&last,&loop); CHECK_RESULT("resetinit"); // restart entire chain
  if( last!=2 || loop!=1 ) { result= aoresult_sys_wrongtopo; CHECK_RESULT("resetinit()"); } // Works on OSP32 with only the two onboard SAIDs
  result= aoosp_send_clrerror(0x000); CHECK_RESULT("clrerror(000)"); // SAIDs have the V flag after boot, clear those
  result= aoosp_send_goactive(0x000); CHECK_RESULT("goactive(000)"); // Switch all nodes (broadcast) to active (allowing to switch on LEDs).
  result= aoosp_send_setpwmchn(0x000,0,0x0FFF,0x0FFF,0x0FFF); CHECK_RESULT("setpwmchn(000,0)"); // All SAIDs: RGB0 white
  result= aoosp_send_setpwmchn(0x000,1,0x0FFF,0x0FFF,0x0FFF); CHECK_RESULT("setpwmchn(000,1)"); // All SAIDs: RGB1 white
  result= aoosp_send_setpwmchn(0x000,2,0x0FFF,0x0FFF,0x0FFF); CHECK_RESULT("setpwmchn(000,2)"); // All SAIDs: RGB2 white

  // Wait
  //   A LOS meeting is performed during the "high" phase of a PWM period. 
  //   We need to wait one entire PWM period to ensure the LOS meeting could 
  //   start and is completed. Therefore, the wait time is 2**15 ticks of the 
  //   internal clock (19.2MHz) or 1.7ms (alternatively 2**14 for fast mode;  
  //   see SETUP.FAST_PWM). The wait is needed after GOACTIVE, but also after 
  //   READLEDSTCHN since that telegram clears the LOS flags. Also note that 
  //   the PWM "high" phase needs to be long enough: 512 internal clocks, so 
  //   PWM value must be greater than 0x0200.
  delay(2);

  // Inspect node 002, this LED has B1 on a jumper
  print_said_ledst(0x002);
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_ledst.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  demo();
}


void loop() {
  delay(5000);
}
