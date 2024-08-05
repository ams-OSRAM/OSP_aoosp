// aoosp_crc.ino - compute and check telegram CRCs
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
#include <aoosp.h>       // generic include for whole aoosp lib
//#include <aoosp_crc.h> // specific include for just CRC


/*
DESCRIPTION
This demo computes the CRC of a telegram (byte buffer).
Once to compute the required CRC for sending a command.
Once to check the CRC in an incoming response.

This is just a CRC example, no telegrams are send/received.

HARDWARE
The demo runs on the OSP32 board, no demo board needs to be attached.
In Arduino select board "ESP32S3 Dev Module".
Since this demo does not use any OSP related hardware, any ESP32[S3] will do.

OUTPUT
Welcome to aoosp_crc.ino
buf A0 07 CF 00 FF 08 88 00 11 08 88 00=94 ERROR
buf A0 06 07 00 00 00 40 AA=AA OK
*/


void check(const uint8_t * buf, int size) {
  uint8_t crc = aoosp_crc(buf, size-1);
  // Print buffer and CRC
  Serial.printf("buf");
  for( int i=0; i<size-1; i++) Serial.printf(" %02X",buf[i]);
  Serial.printf(" %02X=%02X",buf[size-1],crc);
  // Result
  int ok= buf[size-1] == crc;
  Serial.printf(" %s\n", ok?"OK":"ERROR" );
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_crc.ino\n");
}


const uint8_t setpwmchn[] = { 0xA0, 0x07, 0xCF, 0x00, 0xFF, 0x08, 0x88, 0x00, 0x11, 0x08, 0x88, /*CRC placeholder*/ 0x00 };
const uint8_t identify [] = { 0xA0, 0x06, 0x07, 0x00, 0x00, 0x00, 0x40, 0xAA };


void loop() {
  check(setpwmchn, sizeof setpwmchn);
  check(identify , sizeof identify );
  Serial.printf("\n");
  delay(5000);
}
