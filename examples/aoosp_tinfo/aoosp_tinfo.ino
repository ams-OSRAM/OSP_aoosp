// aoosp_tinfo.ino - demonstrates how serial cast works (with asktinfo)
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
This demo shows how to use the ASKTINFO feature. The asktinfo telegram 
collects the minimum and maximum temperature over an OSP chain using a 
mechanism called "serial cast". This demo also explains serial cast.

Serial cast is a like broadcast: the telegram is sent to a node, the node 
does not respond but modifies and forwards the telegram to the next node. 
In the case of asktinfo, it is the last node in the chain that no longer 
forwards but sends a response.
 
When a node receives asktinfo, it measures the temperature. It takes the min 
temperature from the payload of the incoming telegram and mins that with 
the measured temperature. It takes the max temperature from the payload of 
the incoming telegram and maxes that with the temperature. In other words 
the min and max from the incoming telegram are updated with the temperature 
measured by the node. The updated values are put in the outgoing telegram.
 
What this means is that the telegram payload contains two bytes, the min and 
max temperature. However, for convenience, it is allowed that the payload 
is empty. That is typically used for the telegram that the mcu sends to 
the first node. The node that receives a telegram without payload does not 
min/max the incoming values with its measurement, but just inserts them for 
the outgoing telegram. 

HARDWARE
The demo requires a chain with SAIDs only (either in BiDir or Loop), but
more than one for serial cast to make sense. Suggested setup is OSP32
with SAIDlooker: have a cable from the OSP32 OUT to SAIDlooker IN and
from SAIDlooker IN to OSP32 IN.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
This demo reads temperature, so nothing can be seen on the LEDs. 
The output of this demo is on Serial. 
The demo consists of three rounds that each determine the minimum and 
maximum temperatures. First round loops over all nodes using READTEMP
to obtain temperature; the mining/maxing happens on the ESP. The second
round uses ASKTINFO serial cast; the mining/maxing happens in the SAIDs.
The third round shows that the MCY could provide the inital values.
This is an explanatory round (a fake initial value is passed for 
illustrative purposes).

OUTPUT
Welcome to aoosp_tinfo.ino
version: result 0.4.4 spi 0.5.6 osp 0.4.5
spi: init
osp: init

UNICAST
node temp tmin tmax
001  70   70   70
002  6D   6D   70
003  6E   6D   70
004  73   6D   73
005  6F   6D   73
Raw       6D   73
Celsius   23   29

SERIALCAST (none none)
Raw       6E   73
Celsius   24   29

SERIALCAST (64 00) - note tmin should have initial val of FF
Raw       64   73
Celsius   14   29
*/


// Print result if it is not ok
#define CHECK_RESULT(msg) do { if( result!=aoresult_ok ) Serial.printf("ERROR %d in %s (%s)\n", result, msg, aoresult_to_str(result) ); } while( 0 )


void demo_tinfo() {
  aoresult_t result;
  uint16_t   last;
  // Set up OSP chain
  result= aoosp_exec_resetinit(&last); CHECK_RESULT("resetinit"); // restart entire chain
  result= aoosp_send_clrerror(0x000); CHECK_RESULT("clrerror(000)"); // SAIDs have the V flag after boot, clear those
  result= aoosp_send_goactive(0x000); CHECK_RESULT("goactive(000)"); // Switch all nodes (broadcast) to active (allowing to switch on LEDs).

  // Loop over all nodes, get temperature and track min and max
  Serial.printf("\nUNICAST\n");
  uint8_t tmin=0xFF;
  uint8_t tmax=0x00;;
  Serial.printf("node temp tmin tmax\n");
  for( uint16_t addr=0x001; addr<=last; addr++ ) {
    // Check node is a SAID
    uint32_t id;
    result= aoosp_send_identify(addr, &id ); CHECK_RESULT("identify()"); 
    if( !AOOSP_IDENTIFY_IS_SAID(id) ) { result= aoresult_sys_id; CHECK_RESULT("identify(addr)"); }
    // Get node's (raw) temperature
    uint8_t temp;
    result= aoosp_send_readtemp(addr, &temp); CHECK_RESULT("readtemp(addr)"); 
    // Track min and max;
    if( temp<tmin ) tmin= temp;
    if( temp>tmax ) tmax= temp;
    Serial.printf("%03X  %02X   %02X   %02X\n",addr,temp,tmin,tmax);
  }
  Serial.printf("Raw       %02X   %02X\n", tmin, tmax );
  Serial.printf("Celsius %4d %4d\n", aoosp_prt_temp_said(tmin), aoosp_prt_temp_said(tmax) );

  // Use serialcast to get tmin and tmax
  Serial.printf("\nSERIALCAST (none none)\n");
  result= aoosp_send_asktinfo(0x001, &tmin, &tmax ); CHECK_RESULT("gettinfo(001)"); 
  Serial.printf("Raw       %02X   %02X\n", tmin, tmax );
  Serial.printf("Celsius %4d %4d\n", aoosp_prt_temp_said(tmin), aoosp_prt_temp_said(tmax) );

  // Use serialcast to get tmin and tmax (but now with initial values)
  tmin= max(0x00,tmin-10); // Should be 0xFF - this is just to show that an initial value can be passed
  tmax= 0x00;
  Serial.printf("\nSERIALCAST (%02X %02X) - note tmin should have initial val of FF\n", tmin, tmax);
  result= aoosp_send_asktinfo_init(0x001, &tmin, &tmax ); CHECK_RESULT("gettinfo_init(001)"); 
  Serial.printf("Raw       %02X   %02X\n", tmin, tmax );
  Serial.printf("Celsius %4d %4d\n", aoosp_prt_temp_said(tmin), aoosp_prt_temp_said(tmax) );
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_tinfo.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  demo_tinfo();
}


void loop() {
  delay(5000);
}
