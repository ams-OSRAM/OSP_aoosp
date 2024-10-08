// aoosp_cur.ino - select the drive current of SAID channels
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
This demo configures channel 1 of node 001 to use a high drive current,
and it configures channel 1 of node 002 to use a low drive current.
Next it broadcasts setpwm with max brightness (7FFF 7FFF 7FFF). This
results in the two white lights with different brightness.

HARDWARE
The demo should run on the OSP32 board.
Have a cable from the OUT connector to the IN connector. 
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
The second RGB of SAID OUT (L1.1) shall be bright white.
The second RGB of SAID IN (L2.1) shall be dim white.
This is caused by different drive current settings, because
the setpwm is braodcast.

OUTPUT
Welcome to aoosp_cur.ino
spi: init
osp: init

resetinit ok 2
clrerror ok
goactive ok
setcurchn(001,1,hi) ok
setcurchn(002,1,lo) ok
setpwmchn(000,1,max) ok
*/


void setcur() {
  aoresult_t result;
  uint16_t   last;

  // Reset and init
  result= aoosp_exec_resetinit(&last); 
  Serial.printf("resetinit %s %d\n", aoresult_to_str(result), last );
  if( last!=2 ) Serial.printf("demo expects OSP32.OUT to be connected to OSP32.IN\n");

  // Clear the error flags of all node (broadcast).
  result= aoosp_send_clrerror(0x000); 
  Serial.printf("clrerror %s\n", aoresult_to_str(result) );

  // Switch the state of all nodes (broadcast) to active (allowing to switch on LEDs).
  result= aoosp_send_goactive(0x000);
  Serial.printf("goactive %s\n", aoresult_to_str(result) );

  // Set SAID.OUT.CHN1 (all 3 lines) to high driver current (4=24mA)
  result= aoosp_send_setcurchn(0x001,1/*chn*/,AOOSP_CURCHN_CUR_DEFAULT,4,4,4);
  Serial.printf("setcurchn(001,1,hi) %s\n", aoresult_to_str(result) );

  // Set SAID.IN.CHN1 (all 3 lines) to low drive current (0=1.5mA)
  result= aoosp_send_setcurchn(0x002,1/*chn*/,AOOSP_CURCHN_CUR_DEFAULT,0,0,0);
  Serial.printf("setcurchn(002,1,lo) %s\n", aoresult_to_str(result) );

  // Set three PWM values of channel 1 to max - for all nodes (broadcast)
  result= aoosp_send_setpwmchn(0x000, 1/*chn*/, 0x7FFF/*red*/, 0x7FFF/*green*/, 0x7FFF/*blue*/);
  Serial.printf("setpwmchn(000,1,max) %s\n", aoresult_to_str(result) );

  // NOTE: SAID channel 0 has a more powerfull driver than channel 1 or 2;
  // with the same current setting it is twice as bright.
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_cur.ino\n");

  aospi_init();
  aoosp_init();
  Serial.printf("\n");

  setcur();
}


void loop() {
  delay(5000);
}
