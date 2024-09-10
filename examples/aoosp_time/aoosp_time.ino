// aoosp_time.ino - measuring telegram timing
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
This demo sends a series of telegrams. The series originally comes from 
aomw_topo_build. We measure how long it takes to run that series.
Once we use the high level aoosp API, and once we use low level aospi API.
The conclusion is that the software overhead can be ignored.

HARDWARE
The demo should run on the OSP32 board with the OUT connected to the 
SAIDbasic demo board, where SAIDbasic its OUT connects the OSP32.IN.
In Arduino select board "ESP32S3 Dev Module".

OUTPUT
Welcome to aoosp_time.ino
version: result 0.1.7 spi 0.2.4 osp 0.1.13
spi: init
osp: init

test_spi:time 5117us commands 42 responses 15
test_osp:time 5072us commands 42 responses 15

test_spi:time 4958us commands 42 responses 15
test_spi:time 4941us commands 42 responses 15
test_spi:time 4934us commands 42 responses 15
test_osp:time 4971us commands 42 responses 15
test_osp:time 4991us commands 42 responses 15
test_osp:time 4976us commands 42 responses 15

ANALYSES
In total 42 commands were sent, and 15 responses received, or 57 telegrams in total.
At 2.4MHz, one 12 bytes (96 bit) telegram takes 40us, so 57 would take 2280us.
OSP (high level) sends these in ~4980us, and SPI (low level) in ~4940us.
This difference between high level and low level (software overhead) can be ignored. 
But practice (5000us) is twice longer than the theory (2300us).

We did overestimate payload size (always 12 bytes), but we did not take into accont 
inter telegram time, telegram forwarding time (along the chain) or telegram 
execution time (reset, otp). 
*/


void test_diag() {
  uint16_t last;
  uint8_t temp;
  uint8_t stat;
  aoresult_t result;
  #define CHECK() do { if( result!=aoresult_ok ) Serial.printf("ERROR %s\n",aoresult_to_str(result)); } while(0)
  aospi_dirmux_set_loop();
  result= aoosp_send_reset(0x000); CHECK();
  delayMicroseconds(150);
  result= aoosp_send_initloop(0x001, &last, &temp, &stat); CHECK();
  if( last!=9 ) { Serial.printf("WARNING: is SAIDbasic connected in loop?\n"); delay(5000); }
}


void test_spi() {
  uint8_t resp[12];
  aoresult_t result;
  Serial.printf("test_spi:");
  aoosp_loglevel_set(aoosp_loglevel_none); // disable log (for time measurement)
  aospi_txcount_reset();
  aospi_rxcount_reset();
  aospi_dirmux_set_loop();
  #define CHECK() do { if( result!=aoresult_ok ) Serial.printf("ERROR %s\n",aoresult_to_str(result)); } while(0)
  uint32_t time0us=micros();
  { const uint8_t buf[]= {0xA0,0x00,0x00,0x22};                result= aospi_tx( buf, sizeof buf ); }              CHECK(); // reset
  delayMicroseconds(150);
  { const uint8_t buf[]= {0xA0,0x04,0x03,0x86};                result= aospi_txrx( buf, sizeof buf, resp, 4+2 ); } CHECK(); // initloop
  { const uint8_t buf[]= {0xA0,0x04,0x07,0x3A};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x04,0xD8,0x0D,0xE2};           result= aospi_txrx( buf, sizeof buf, resp, 4+8 ); } CHECK(); // readotp
  { const uint8_t buf[]= {0xA0,0x08,0x07,0x6A};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x08,0xD8,0x0D,0xAA};           result= aospi_txrx( buf, sizeof buf, resp, 4+8 ); } CHECK(); // readotp
  { const uint8_t buf[]= {0xA0,0x0C,0x07,0xBF};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x0C,0xD8,0x0D,0x92};           result= aospi_txrx( buf, sizeof buf, resp, 4+8 ); } CHECK(); // readotp
  { const uint8_t buf[]= {0xA0,0x10,0x07,0xCA};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x14,0x07,0x1F};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x14,0xD8,0x0D,0x02};           result= aospi_txrx( buf, sizeof buf, resp, 4+8 ); } CHECK(); // readotp
  { const uint8_t buf[]= {0xA0,0x18,0x07,0x4F};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x1C,0x07,0x9A};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x20,0x07,0xA5};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x24,0x07,0x70};                result= aospi_txrx( buf, sizeof buf, resp, 4+4 ); } CHECK(); // identify
  { const uint8_t buf[]= {0xA0,0x24,0xD8,0x0D,0x0D};           result= aospi_txrx( buf, sizeof buf, resp, 4+8 ); } CHECK(); // readotp
  { const uint8_t buf[]= {0xA0,0x00,0x01,0x0D};                result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // clrerror
  { const uint8_t buf[]= {0xA0,0x04,0xCD,0x33,0x93};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x08,0xCD,0x33,0xDB};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x0C,0xCD,0x33,0xE3};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x10,0xCD,0x33,0x4B};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x14,0xCD,0x33,0x73};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x18,0xCD,0x33,0x3B};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x1C,0xCD,0x33,0x03};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x20,0xCD,0x33,0x44};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x24,0xCD,0x33,0x7C};           result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setsetup
  { const uint8_t buf[]= {0xA0,0x15,0xD1,0x02,0x04,0x44,0x0B}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x05,0xD1,0x00,0x12,0x22,0xB4}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x05,0xD1,0x01,0x13,0x33,0xD2}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x05,0xD1,0x02,0x13,0x33,0xC0}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x09,0xD1,0x00,0x12,0x22,0xA5}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x09,0xD1,0x01,0x13,0x33,0xC3}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x09,0xD1,0x02,0x13,0x33,0xD1}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x0D,0xD1,0x00,0x12,0x22,0x4F}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x0D,0xD1,0x01,0x13,0x33,0x29}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x0D,0xD1,0x02,0x13,0x33,0x3B}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x15,0xD1,0x00,0x12,0x22,0x6D}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x15,0xD1,0x01,0x13,0x33,0x0B}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x25,0xD1,0x00,0x12,0x22,0x29}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x25,0xD1,0x01,0x13,0x33,0x4F}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x25,0xD1,0x02,0x13,0x33,0x5D}; result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // setcurchn
  { const uint8_t buf[]= {0xA0,0x00,0x05,0xB1};                result= aospi_tx  ( buf, sizeof buf ); }            CHECK(); // goactive
  uint32_t time1us=micros();
  Serial.printf("time %luus commands %d responses %d\n",time1us-time0us,aospi_txcount_get(), aospi_rxcount_get() );
}


void test_osp() {
  uint16_t last;
  uint8_t temp;
  uint8_t stat;
  uint32_t id;
  uint8_t otp;
  aoresult_t result;
  Serial.printf("test_osp:");
  aoosp_loglevel_set(aoosp_loglevel_none); // disable log (for time measurement)
  aospi_txcount_reset();
  aospi_rxcount_reset();
  aospi_dirmux_set_loop();
  #define CHECK() do { if( result!=aoresult_ok ) Serial.printf("ERROR %s\n",aoresult_to_str(result)); } while(0)
  uint32_t time0us=micros();
    result= aoosp_send_reset(0x000); CHECK();
    delayMicroseconds(150);
    result= aoosp_send_initloop(0x001, &last, &temp, &stat); CHECK();
    result= aoosp_send_identify(0x001,&id); CHECK();
    result= aoosp_send_readotp(0x001,0x0D,&otp,1); CHECK();
    result= aoosp_send_identify(0x002,&id); CHECK();
    result= aoosp_send_readotp(0x002,0x0D,&otp,1); CHECK();
    result= aoosp_send_identify(0x003,&id); CHECK();
    result= aoosp_send_readotp(0x003,0x0D,&otp,1); CHECK();
    result= aoosp_send_identify(0x004,&id); CHECK();
    result= aoosp_send_identify(0x005,&id); CHECK();
    result= aoosp_send_readotp(0x005,0x0D,&otp,1); CHECK();
    result= aoosp_send_identify(0x006,&id); CHECK();
    result= aoosp_send_identify(0x007,&id); CHECK();
    result= aoosp_send_identify(0x008,&id); CHECK();
    result= aoosp_send_identify(0x009,&id); CHECK();
    result= aoosp_send_readotp(0x009,0x0D,&otp,1); CHECK();
    result= aoosp_send_clrerror(0x000); CHECK();
    result= aoosp_send_setsetup(0x001,0x33); CHECK();
    result= aoosp_send_setsetup(0x002,0x33); CHECK();
    result= aoosp_send_setsetup(0x003,0x33); CHECK();
    result= aoosp_send_setsetup(0x004,0x33); CHECK();
    result= aoosp_send_setsetup(0x005,0x33); CHECK();
    result= aoosp_send_setsetup(0x006,0x33); CHECK();
    result= aoosp_send_setsetup(0x007,0x33); CHECK();
    result= aoosp_send_setsetup(0x008,0x33); CHECK();
    result= aoosp_send_setsetup(0x009,0x33); CHECK();
    result= aoosp_send_setcurchn(0x005,2,AOOSP_CURCHN_FLAGS_DEFAULT,4,4,4); CHECK();
    result= aoosp_send_setcurchn(0x001,0,AOOSP_CURCHN_FLAGS_DITHER,2,2,2); CHECK();
    result= aoosp_send_setcurchn(0x001,1,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x001,2,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x002,0,AOOSP_CURCHN_FLAGS_DITHER,2,2,2); CHECK();
    result= aoosp_send_setcurchn(0x002,1,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x002,2,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x003,0,AOOSP_CURCHN_FLAGS_DITHER,2,2,2); CHECK();
    result= aoosp_send_setcurchn(0x003,1,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x003,2,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x005,0,AOOSP_CURCHN_FLAGS_DITHER,2,2,2); CHECK();
    result= aoosp_send_setcurchn(0x005,1,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x009,0,AOOSP_CURCHN_FLAGS_DITHER,2,2,2); CHECK();
    result= aoosp_send_setcurchn(0x009,1,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_setcurchn(0x009,2,AOOSP_CURCHN_FLAGS_DITHER,3,3,3); CHECK();
    result= aoosp_send_goactive(0x000); CHECK();
  uint32_t time1us=micros();
  Serial.printf("time %luus commands %d responses %d\n",time1us-time0us,aospi_txcount_get(), aospi_rxcount_get() );
}


uint32_t time0us,time1us;
void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_time.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();
  Serial.printf("\n");

  // Check for right topo
  test_diag();

  // Flush the ESP caches
  test_spi(); // measure timing os spi level
  test_osp(); // measure timing on osp level
  Serial.printf("\n");

  // Run timing test on SPI level
  test_spi();
  test_spi();
  test_spi();

  // Run timing test on OSP level
  test_osp();
  test_osp();
  test_osp();
}

int report;
void loop() {
  delay(1000);
}


/*
Run time of other tests
----------------------
test_spi
time 4961us commands 42 responses 15

test_osp
time 4993us commands 42 responses 15

time 5260us commands 42 responses 15

*/
