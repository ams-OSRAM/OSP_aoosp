// aoosp_adcled.ino - demo how use the built-in ADC to measure Vf of LEDs
/*****************************************************************************
 * Copyright 2025 by ams OSRAM AG                                            *
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
#include <aoresult.h>
#include <aospi.h>
#include <aoosp.h>


/*
DESCRIPTION
This sketch uses the ADC inside the SAID to measure the forward voltage
of the LEDs connected to it.

HARDWARE
The demo assumes an OSP32 board with OUT looped back to IN.
In Arduino select board "ESP32S3 Dev Module".
See https://github.com/ams-OSRAM/OSP_aotop/tree/main/extras/manuals/saidversions
for differences in ADC channel numbering over SID versions.

BEHAVIOR
The sketch switches on the first two RGBs of SAID OUT (they light up white). 
During that time all six LEDs are connected to the ADC, one by one, to 
measure their forward voltage (several times). The averaged result is printed 
on Serial. Then the RGBs are sent to sleep; they switch off.

OUTPUT
Welcome to aoosp_adcled.ino
version: result 0.4.5 spi 0.5.8 osp 0.7.0
spi: init
osp: init

Welcome to aoosp_adcled.ino
version: result 0.4.6 spi 0.5.9 osp 0.7.0
spi: init
osp: init

Measured (10x) at 24mA and 100% PWM
  1/R0: 607 604 604 605 604 604 604 604 605 605 => 2107 mV
  7/G0: 755 755 755 755 755 755 755 755 755 755 => 2631 mV
  4/B0: 820 820 820 820 820 819 820 820 820 820 => 2858 mV
  2/R1: 605 605 604 604 605 605 605 605 605 605 => 2107 mV
  8/G1: 750 750 750 750 750 750 750 750 750 750 => 2614 mV
  5/B1: 827 827 827 826 827 826 827 826 827 826 => 2883 mV
*/


// Address of the SAID whose Vf we measure
#define TARGET_ADDR    0x001  // SAID OUT on OSP32 (ALERT: make sure to select a SAID version v1.1, not v1.0)
#define COUNT          10     // Averaging ADC over COUNT samples
#define PWM            0xFFFF // Highest PWM gives highest accuracy (for ADC measurement)
// See https://github.com/ams-OSRAM/OSP_aotop/tree/main/extras/manuals/saidversions


// Simple error handling
#define CHECKRESULT(tele) do { if( result!=aoresult_ok ) { Serial.printf("ERROR in " tele ": %s\n", aoresult_to_str(result) ); return; } } while(0)


void adc_test() {
  aoresult_t result;
  uint16_t   adcdat;
  uint32_t   id;

  // Reset and init all nodes, clr error flags, activate, then switch on one triplet (white) on 24 mA.
  result= aoosp_exec_resetinit(); CHECKRESULT("resetinit");
  result= aoosp_send_identify(TARGET_ADDR,&id); 
  if( result!=aoresult_ok || !AOOSP_IDENTIFY_IS_SAID(id) ) { Serial.printf("Need SAID at %03X\n",TARGET_ADDR); return; }
  result= aoosp_send_clrerror(0x000); CHECKRESULT("clrerror");
  result= aoosp_send_goactive(0x000); CHECKRESULT("goactive");
  // Set the first 6 drivers (of SAID OUT) to 24 mA drive current
  result= aoosp_send_setcurchn(0x000, 0, AOOSP_CURCHN_FLAGS_DEFAULT, 3,3,3); CHECKRESULT("setcurchn0");
  result= aoosp_send_setcurchn(0x000, 1, AOOSP_CURCHN_FLAGS_DEFAULT, 4,4,4); CHECKRESULT("setcurchn1");
  // Set all PWMs to max (below 0x0600 the ADC fails, the higher the more accurate)
  result= aoosp_send_setpwmchn(TARGET_ADDR, 0, PWM, PWM, PWM); CHECKRESULT("setpwmchn0");
  result= aoosp_send_setpwmchn(TARGET_ADDR, 1, PWM, PWM, PWM); CHECKRESULT("setpwmchn0");

  // Measure and print results
  Serial.printf("\n");
  Serial.printf("Measured (%dx) at 24mA and %d%% PWM\n",COUNT, PWM*100/0xFFFF );
  // Skipping channel 2 of SAID OUT, because it has I2C
  for( int chn=0; chn<2; chn++ ) for( int col=0; col<3; col++ /*0=R,1=G,2=B*/ )  { 
    // Compute ADC mux input from channel and color
    int adcmux = AOOSP_ADC_FLAGS_MUX_DRVcc(col,chn); 
    // Work around: _changing_ the mux input locks it. To unlock, temporarily disable syncing.
    result= aoosp_send_setadc(TARGET_ADDR, AOOSP_ADC_FLAGS_SYNC_DIS+adcmux ); CHECKRESULT("setadc-dis");
    // Set the ADC mux to a new input `adcmux`
    result= aoosp_send_setadc(TARGET_ADDR, AOOSP_ADC_FLAGS_SYNC_ENA+adcmux ); CHECKRESULT("setadc-ena");
    // Perform COUNT measurements
    Serial.printf("  %d/%s:", adcmux, aoosp_prt_adcmux(adcmux) ); 
    int sum=0;
    for( int i=0; i<COUNT; i++ ) {
      delay(4); // wait two PWM cycles (one is enough except for the first after SETADC, that needs two)
      result= aoosp_send_readadcdat(TARGET_ADDR, &adcdat); CHECKRESULT("readadcdat");
      Serial.printf(" %d", adcdat ); 
      sum+= adcdat;
    }
    Serial.printf(" => %d mV\n", aoosp_prt_adc((sum+COUNT/2)/COUNT) );
  }

  // Switch off LEDs, but wait a bit otherwise too much flash
  delay(2000);
  result= aoosp_send_setadc(TARGET_ADDR, AOOSP_ADC_FLAGS_SYNC_DIS+AOOSP_ADC_FLAGS_MUX_AUTO ); CHECKRESULT("setadc-auto");
  result= aoosp_send_gosleep(0x000); CHECKRESULT("gosleep");
}


void setup() {
  Serial.begin(115200); 
  Serial.printf("\n\nWelcome to aoosp_adcled.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  adc_test();
}


void loop() {
  delay(5000);
}


