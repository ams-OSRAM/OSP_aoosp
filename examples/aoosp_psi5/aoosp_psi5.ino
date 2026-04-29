// aoosp_psi5.ino - demo testing compatibility of PSI (payload size) of 5
/*****************************************************************************
 * Copyright 2026 by ams OSRAM AG                                            *
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
This demo shows that telegrams with a PSI of 5 are not forwarded by OSP V1.0
nodes like the RGBIs on SAIDbasic demo board. 
This is demonstrated by sending an I2C write telegram to a SAID, asking for
an I2C write transaction to a non-existing I2C device. This should normally 
fail with a NACK in the SAID's I2C config register. 
When we send such a telegram with few bytes to write, the telegram has a 
PSI of 3, and indeed we get a NACK. When we send such a telegram with more 
bytes to write, the telegram has a PSI of 5. If there is an RGBI before the 
SAID with the I2C bridge, the RGBI blocks the telegram, the SAID never 
receives it, the I2C transaction is not executed and the NACK flag is not set.
Both telegrams are sent by this example.

When this demo would be run with SAIDsense instead of SAIDbasic (change macros 
LAST to 3 and ADDR to 0x003), there are no RGBIs in the chain, both I2C write 
telegrams are forwarded to the SAID, and Serial shows twice NACK.

HARDWARE
The demo assumes an OSP32 board connected to SAIDbasic and then a terminator.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
Nothing to see, check Serial.

OUTPUT
Welcome to aoosp_psi5.ino
version: result 0.5.0 spi 1.0.1 osp 0.9.0
spi: init(MCU-B)
osp: init

I2CWRITE to a non-existing I2C device (PSI=5)
I2CCFG=00=itnb
NACK is not set: telegram did not arrive
Expected on SAIDbasic (telegrams with PSI=5 are blocked, no I2C transaction, no NACK)

I2CWRITE to a non-existing I2C device (PSI=3)
I2CCFG=02=itNb
NACK is set: telegram did arrive but failed
Expected on SAIDbasic (telegrams with PSI=3 are forwarded, I2C device is not present, results in NACK)
*/


// Simple error handling
#define CHECKRESULT(tele) do { if( result!=aoresult_ok ) { Serial.printf("ERROR in " tele ": %s\n", aoresult_to_str(result) ); return; } } while(0)


// OSP node address of the SAID to use for I2C
#define LAST              8 // SAIDbasic in bidir
#define ADDR          0x005 // SAIDbasic has SAID with I2C bridge on addr 005
#define DADDR7_FAKE    0x03 // I2C device address - one which does _not_ exist
#define REG_FAKE       0x33 // A register address in the non existing I2C device


// Print a table of all I2C device addresses that acknowledge a read
void i2c_psi5() {
  uint8_t    buf1[] = {1,2,3}; // with I2C daddr7 and register raddr results in payload of 5 bytes
  uint8_t    buf2[] = {1};
  uint8_t    flags;
  uint8_t    speed;
  aoresult_t result;

  // Reset chain and power I2C bus
  result= aoosp_exec_resetinit(); CHECKRESULT("aoosp_exec_resetinit");
  if( aoosp_exec_resetinit_last()!=LAST ) Serial.printf("ERROR: topology mismatch, expected OSP32-SAIDbasic-terminator\n");
  result= aoosp_exec_i2cpower(ADDR); CHECKRESULT("aoosp_exec_i2cpower");

  // Test 1
  Serial.printf("\nI2CWRITE to a non-existing I2C device (PSI=5)\n");
  result = aoosp_send_i2cwrite8(ADDR, DADDR7_FAKE, REG_FAKE, buf1, sizeof buf1); CHECKRESULT("aoosp_send_i2cwrite8()");
  delay(10); // Wait for I2C transaction to complete
  result = aoosp_send_readi2ccfg(ADDR,&flags,&speed); CHECKRESULT("aoosp_send_readi2ccfg()");
  Serial.printf("I2CCFG=%02x=%s\n",flags,aoosp_prt_i2ccfg(flags));
  if( flags==AOOSP_I2CCFG_FLAGS_NACK ) {
    Serial.printf("NACK is set: telegram did arrive but failed\n"); 
    Serial.printf("**Not expected** SAIDbasic (telegrams with PSI=5 are blocked, no I2C transaction, can be no NACK)\n"); 
  } else {
    Serial.printf("NACK is not set: telegram did not arrive\n"); 
    Serial.printf("Expected on SAIDbasic (telegrams with PSI=5 are blocked, no I2C transaction, no NACK)\n"); 
  }

  // Test 2
  Serial.printf("\nI2CWRITE to a non-existing I2C device (PSI=3)\n");
  result = aoosp_send_i2cwrite8(ADDR, DADDR7_FAKE, REG_FAKE, buf2, sizeof buf2); CHECKRESULT("aoosp_send_i2cwrite8()");
  delay(10); // Wait for I2C transaction to complete
  result = aoosp_send_readi2ccfg(ADDR,&flags,&speed); CHECKRESULT("aoosp_send_readi2ccfg()");
  Serial.printf("I2CCFG=%02x=%s\n",flags,aoosp_prt_i2ccfg(flags));
  if( flags==AOOSP_I2CCFG_FLAGS_NACK ) {
    Serial.printf("NACK is set: telegram did arrive but failed\n"); 
    Serial.printf("Expected on SAIDbasic (telegrams with PSI=3 are forwarded, I2C device is not present, results in NACK)\n"); 
  } else { 
    Serial.printf("NACK is not set: telegram did not arrive\n"); 
    Serial.printf("**Not expected** on SAIDbasic (telegrams with PSI=3 are forwarded, I2C device is not present, there must be NACK)\n"); 
  }

}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_psi5.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  aospi_warnings_set(false); // suppress PSI=5 warning, comment out to get warning
  i2c_psi5();
}


void loop() {
  delay(1000);
}
