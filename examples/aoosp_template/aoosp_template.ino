// aoosp_template.ino - template sketch for a short sequence of OSP telegrams.
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
A template sketch for a short sequence of OSP telegrams.

HARDWARE
The demo should run on the OSP32 board; with terminator in OUT.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
The first RGB (L1.0 aka OUT0) will turn red.

OUTPUT
Welcome to aoosp_template.ino
version: result 0.5.0 spi 1.0.1 osp 0.9.0
spi: init(MCU-B)
osp: init
done
*/


// Simple `result` checking
#define CHECK() do { if(result!=0) Serial.printf("error %d\n",result); } while(0)


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_template.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );
  
  aospi_init();
  aoosp_init();

  aoresult_t result;
  result= aoosp_exec_resetinit(); CHECK(); // "exec" macro; "send" equivalent listed at end of file
  result= aoosp_send_clrerror(0x000); CHECK(); 
  result= aoosp_send_goactive(0x000); CHECK();
  result= aoosp_send_setpwmchn(0x001, 0/*chn*/, 0x3333/*red*/, 0x0000/*green*/, 0x0000/*blue*/); CHECK();

  Serial.printf("done\n");
}


void loop() {
  delay(5000);
}


// uint16_t last;
// uint8_t temp;
// uint8_t stat;
// result= aoosp_send_reset(0x000); CHECK();
// result= aoosp_send_initbidir(0x001,&last,&temp,&stat); CHECK(); // bidir is default, see aospi_dirmux_set_loop();
