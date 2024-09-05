// aoosp_error.ino - show the effect of error trapping 
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
This demo shows how error handling works in SAID. The STAT register has 
error flags denoting which errors occurred. The SETUP register has flags
denoting which errors need to be trapped. When an error occurs 
(is flagged in STAT) whose trap bit is set (in SETUP), then the error
is handled by switching to SLEEP state. This switches off the PWMs.

Note that the communication-error (whose trap bit AOOSP_SETUP_FLAGS_CE is 
set in this demo) is a bit special: it can be triggered by malformed 
telegram. One way to create a malformed telegram is to change the CRC of 
a well formed telegram. Note however, that SAIDs only check CRC when they 
are setup to do so (AOOSP_SETUP_FLAGS_CRCEN).

HARDWARE
The demo runs on the OSP32 board. Have a cable from the OUT connector to 
the IN connector so that both SAIDs are in the chain.
In Arduino select board "ESP32S3 Dev Module".

OUTPUT
Welcome to aoosp_error.ino
version: result 0.4.0 spi 0.5.0 osp 0.4.0

spi: init
osp: init

DEMO1
condition: CRC checking disabled | CRC trapping disabled | mult 0000 | error flags 00 | status ACTIVE
action   : setmult(1) with ok CRC
condition: CRC checking disabled | CRC trapping disabled | mult 0001 | error flags 00 | status ACTIVE
action   : setmult(2) with bad CRC
condition: CRC checking disabled | CRC trapping disabled | mult 0002 | error flags 00 | status ACTIVE
observe  : with 'CRC checking _disabled_' the bad setmult(2) is just accepted
observe  : no error flags set, status stays ACTIVE, green led stays on

DEMO2
condition: CRC checking enabled | CRC trapping disabled | mult 0000 | error flags 00 | status ACTIVE
action   : setmult(1) with ok CRC
condition: CRC checking enabled | CRC trapping disabled | mult 0001 | error flags 00 | status ACTIVE
action   : setmult(2) with bad CRC
condition: CRC checking enabled | CRC trapping disabled | mult 0001 | error flags 08 | status ACTIVE
observe  : with 'CRC checking _enabled_' the bad setmult(2) is not accepted
observe  : an error flag is set, but status stays ACTIVE, green led stays on

DEMO3
condition: CRC checking enabled | CRC trapping enabled | mult 0000 | error flags 00 | status ACTIVE
action   : setmult(1) with ok CRC
condition: CRC checking enabled | CRC trapping enabled | mult 0001 | error flags 00 | status ACTIVE
action   : setmult(2) with bad CRC
condition: CRC checking enabled | CRC trapping enabled | mult 0001 | error flags 08 | status SLEEP
observe  : with 'CRC checking _enabled_' the bad setmult(2) is not accepted
observe  : with 'CRC trapping _enabled_' the bad setmult(2) is even trapped
observe  : an error flag is set, status moved to SLEEP, green led switches off
*/


// We need to send telegrams with incorrect CRC; we will use aospi_tx for that
const uint8_t setmult_001_01_ok [] = {0xA0, 0x05, 0x0D, 0x00, 0x01,   0xE1}; // the CRC is ok
const uint8_t setmult_001_01_err[] = {0xA0, 0x05, 0x0D, 0x00, 0x01, 1+0xE1}; // the CRC is bad
const uint8_t setmult_001_02_err[] = {0xA0, 0x05, 0x0D, 0x00, 0x02, 1+0x90}; // the CRC is bad
const uint8_t setmult_001_04_err[] = {0xA0, 0x05, 0x0D, 0x00, 0x04, 1+0x72}; // the CRC is bad
const uint8_t setmult_001_08_err[] = {0xA0, 0x05, 0x0D, 0x00, 0x08, 1+0x99}; // the CRC is bad


// Print result if it is not ok
#define CHECK_RESULT(msg) do { if( result!=aoresult_ok ) Serial.printf("ERROR %d in %s (%s)\n", result, msg, aoresult_to_str(result) ); } while( 0 )


void reset_to_green() {
  aoresult_t result;
  result= aoosp_exec_resetinit(); CHECK_RESULT("resetinit"); // restart entire chain
  result= aoosp_send_clrerror(0x000); CHECK_RESULT("clrerror"); // SAIDs have the V flag after boot, clear those
  result= aoosp_send_goactive(0x000); CHECK_RESULT("goactive"); // Switch all nodes (broadcast) to active (allowing to switch on LEDs).
  result= aoosp_send_setpwmchn(0x001, 0/*chn*/, 0x0000/*red*/, 0x7FFF/*green*/, 0x0000/*blue*/); CHECK_RESULT("setpwmchn"); // Set channel 0 of first SAID to green
}


void print_condition() {
  aoresult_t result;
  uint8_t    stat;
  uint8_t    setup;
  uint16_t   groups;
  Serial.printf("condition: ");
  result= aoosp_send_readsetup(0x001, &setup); CHECK_RESULT("readsetup"); 
  Serial.printf("CRC checking %s | ", setup&AOOSP_SETUP_FLAGS_CRCEN ? "enabled" : "disabled" );
  Serial.printf("CRC trapping %s | ", setup&AOOSP_SETUP_FLAGS_CE    ? "enabled" : "disabled" );
  result= aoosp_send_readmult(0x001, &groups); CHECK_RESULT("readmult"); 
  Serial.printf("mult %04X | ", groups );
  result= aoosp_send_readstat(0x001, &stat); CHECK_RESULT("readstat"); 
  Serial.printf("error flags %02X | ", stat&AOOSP_STAT_FLAGS_SAID_ERRORS );
  Serial.printf("status %s\n", aoosp_prt_stat_state(stat) );
}


void demo1() {
  aoresult_t result;
  Serial.printf("\nDEMO1\n");
  reset_to_green();
  print_condition();

  Serial.printf("action   : setmult(1) with ok CRC\n");
  result = aospi_tx( setmult_001_01_ok, sizeof setmult_001_01_ok ); CHECK_RESULT("setmult_001_01_ok"); 
  print_condition();

  Serial.printf("action   : setmult(2) with bad CRC\n");
  result = aospi_tx( setmult_001_02_err, sizeof setmult_001_02_err ); CHECK_RESULT("setmult_001_02_err"); 
  print_condition();

  Serial.printf("observe  : with 'CRC checking _disabled_' the bad setmult(2) is just accepted\n");
  Serial.printf("observe  : no error flags set, status stays ACTIVE, green led stays on\n");
}


void demo2() {
  aoresult_t result;
  Serial.printf("\nDEMO2\n");
  reset_to_green();
  result= aoosp_send_setsetup(0x001,AOOSP_SETUP_FLAGS_SAID_DFLT | AOOSP_SETUP_FLAGS_CRCEN); CHECK_RESULT("setsetup"); 
  print_condition();

  Serial.printf("action   : setmult(1) with ok CRC\n");
  result = aospi_tx( setmult_001_01_ok, sizeof setmult_001_01_ok ); CHECK_RESULT("setmult_001_01_ok"); 
  print_condition();

  Serial.printf("action   : setmult(2) with bad CRC\n");
  result = aospi_tx( setmult_001_02_err, sizeof setmult_001_02_err ); CHECK_RESULT("setmult_001_02_err"); 
  print_condition();

  Serial.printf("observe  : with 'CRC checking _enabled_' the bad setmult(2) is not accepted\n");
  Serial.printf("observe  : an error flag is set, but status stays ACTIVE, green led stays on\n");
}


void demo3() {
  aoresult_t result;
  Serial.printf("\nDEMO3\n");
  reset_to_green();
  result= aoosp_send_setsetup(0x001,AOOSP_SETUP_FLAGS_SAID_DFLT | AOOSP_SETUP_FLAGS_CRCEN | AOOSP_SETUP_FLAGS_CE); CHECK_RESULT("setsetup"); 
  print_condition();

  Serial.printf("action   : setmult(1) with ok CRC\n");
  result = aospi_tx( setmult_001_01_ok, sizeof setmult_001_01_ok ); CHECK_RESULT("setmult_001_01_ok"); 
  print_condition();

  Serial.printf("action   : setmult(2) with bad CRC\n");
  result = aospi_tx( setmult_001_02_err, sizeof setmult_001_02_err ); CHECK_RESULT("setmult_001_02_err"); 
  print_condition();

  Serial.printf("observe  : with 'CRC checking _enabled_' the bad setmult(2) is not accepted\n");
  Serial.printf("observe  : with 'CRC trapping _enabled_' the bad setmult(2) is even trapped\n");
  Serial.printf("observe  : an error flag is set, status moved to SLEEP, green led switches off\n");
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_error.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );
  Serial.printf("\n" );

  aospi_init();
  aoosp_init();

  demo1();
  delay(2000);
  demo2();
  delay(2000);
  demo3();
  delay(2000);
}


void loop() {
  delay(5000);
}

