// aoosp_adcpot.ino - demo how use the built-in ADC to measure an external voltage
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
This sketch configures a SAID pin as ADC input (instead of driving an LED).
Attached to this pad is a potentiometer to vary the voltage. The measured
voltage is printed on Serial and mapped to a color (red to blue) of an
RGB LED.

HARDWARE
The demo runs on the OSP32 board; connect the OUT to the IN (loop back).
Furthermore, connect a (eg 10k) potmeter over GND and VDD (eg from J3C)
and connect the sliding contact of the potmeter to pin S2_B1. This 
requires removing a jumper. See photo connections.jpg in the directory 
for this sketch. Also make sure the correct MUX input is selected, see
#define of TARGET_DRV.
In Arduino select board "ESP32S3 Dev Module".

      +-------------------------+
      | OPS32                   |
  USB |                     OUT o----+
  ----o CMD        SAID IN      |    | loopback cable
      |                 []   IN o----+
      | [+-]         [] [] []   |
      +--++-------------+-------+
        ||             |S2_B1
        ||            \|/
        |+-----############--+
        |        potmeter    |
        +--------------------+

BEHAVIOR
When the potmeter is turned, the left RGB of SAID IN changes from red to blue.
The Serial shows the measured (raw ADC and the) voltage. 
Please note the following
- The ADC (0..1023) has a voltage range of about 3.5V. As a result only 2/3 
  of the potmeter range (assuming 0..5V) the values will change.
- The ADC measures with respect to VDD (not GND!), therefore the measurement
  range is rougly 1.5..5.0.
- When this demo is run from USB CMD, there is a voltage drop caused by
  the protection diode on the ESP32 board. This results in a VDD that is 
  below 5V (assuming an accurate USB power supply). This is "calibrated"
  by setting the macro VDD_mV

OUTPUT
Welcome to aoosp_adcpot.ino
version: result 0.4.5 spi 0.5.8 osp 0.7.0
spi: init
osp: init
TARGET_DRV=6 (see #define to check if it matches your HW)
ADC:430 mV:3265
ADC:429 mV:3268
ADC:429 mV:3268
ADC:430 mV:3265
ADC:429 mV:3268
ADC:429 mV:3268
*/


// Address of the SAID and its ADC channel with the potmeter
#define TARGET_ADDR    0x002 // SAID IN on OSP32
#define TARGET_FBCHN   2     // Feedback channel of that SAID (left-most RGB of SAID IN)
#define TARGET_DRV     6     // SAID version v1.1: 1=R0 2=R1 3=R2 4=B0 5=B1 6=B2 7=G0 8=G1 9=G2 
// On the OSP32 board, pin B1 of SAID IN is available via a jumper.
// That pin is used for the ADC, so we need to set the ADC MUX for that pin.
// Some EVKs have older SAIDs (v1.0) for footprint "SAID IN". 
// Those older versions differ differ in ADC MUX numbering.
// In SAID version V1.0, B1 is connected to MUX 6.
// In SAID version V1.1, B1 is connected to MUX 5.
// See https://github.com/ams-OSRAM/OSP_aotop/tree/main/extras/manuals/saidversions


// Simple error handling
#define CHECKRESULT(tele) do { if( result!=aoresult_ok ) { Serial.printf("ERROR in " tele ": %s\n", aoresult_to_str(result) ); return; } } while(0)


void setup() {
  Serial.begin(115200); 
  Serial.printf("\n\nWelcome to aoosp_adcpot.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();
  Serial.printf("TARGET_DRV=%d (see #define to check if it matches your HW)\n",TARGET_DRV);

  // Bring up chain
  aoresult_t result;
  result= aoosp_exec_resetinit(); CHECKRESULT("aoosp_exec_resetinit");
  if( aoosp_exec_resetinit_last()!=2 ) { Serial.printf("ERROR: demo is written for OSP32 with loop back cable\n"); return; }
  result= aoosp_send_clrerror(0x000); CHECKRESULT("clrerror");
  result= aoosp_send_goactive(0x000); CHECKRESULT("goactive");

  // Measurements _not_ synchronized with PWM (because external pot); ADC mapped to TARGET_DRV.
  // See documentation of `aoosp_send_setadc()` for details on the ADC behavior.
  result= aoosp_send_setadc(TARGET_ADDR, AOOSP_ADC_FLAGS_SYNC_DIS+TARGET_DRV ); CHECKRESULT("setadc");
  // ADC measurement takes 730us, wait that before first readadcdat.
  delay(1); 
}


// ADC measurement is with respect to VDD 
#define VDD_mV (5000-240) // With power supplied via OSP32 its USB CMD, there is a drop due to the protection diode


void loop() {
  uint16_t   adcdat;
  aoresult_t result;
  result= aoosp_send_readadcdat(TARGET_ADDR, &adcdat); CHECKRESULT("readadcdat");
  int mV= VDD_mV - aoosp_prt_adc(adcdat);
  Serial.printf("ADC:%d mV:%d\n", adcdat, mV );
  // Interpolate between red and blue (map 10 to 16 bits); red is brighter than blue, so half its PWM.
  result= aoosp_send_setpwmchn(TARGET_ADDR,TARGET_FBCHN,adcdat*32,0,65535-adcdat*64); CHECKRESULT("setpwmchn");
  delay(50);
}

