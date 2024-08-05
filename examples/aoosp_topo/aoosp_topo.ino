// aoosp_topo.ino - demo showing a simple topological scan (which hardware is where)
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
This demo scans all nodes and prints out the chain configuration:
comms for both SIOs, dir, state, number of RGBs and I2C.

HARDWARE
The demo should run on the OSP32 board. Either have a terminator in the 
OUT connector, have a cable from the OUT connector to the IN connector, 
or connect any OSP demo board in Loop or Bidir mode.
In Arduino select board "ESP32S3 Dev Module".

OUTPUT
Note: the OSP32 board has OUT connected to a SAIDbasic board, which loops back to OSP32 IN

Welcome to aoosp_topo.ino
version: result 0.1.10 spi 0.2.8 osp 0.2.2
spi: init
osp: init

resetinit last 009 loop

 mcu [001 SAID loop sleep][RGB][RGB][I2C] lvds
lvds [002 SAID loop sleep][RGB][RGB][RGB] lvds
lvds [003 SAID loop sleep][RGB][RGB][RGB] lvds
lvds [004 RGBI loop sleep][RGB] lvds
lvds [005 SAID loop sleep][RGB][RGB][I2C] lvds
lvds [006 RGBI loop sleep][RGB] lvds
lvds [007 RGBI loop sleep][RGB] lvds
lvds [008 RGBI loop sleep][RGB] lvds
lvds [009 SAID loop sleep][RGB][RGB][RGB] eol 
*/


void topo() {
  aoresult_t result;
  uint16_t   last;
  int        loop;

  // Reset all nodes, then autodetect direction, and finally init the chain.
  result= aoosp_exec_resetinit(&last,&loop); 
  if( result!=aoresult_ok ) { Serial.printf("ERROR resetinit %s\n", aoresult_to_str(result) ); return; }
  Serial.printf("resetinit last %03X %s\n", last, loop?"loop":"bidir" );
  Serial.printf("\n");

  // In case the SAID does not have the I2C_BRIDGE_EN override this in RAM - just for demo.
  //result= aoosp_exec_i2cenable_set(0x001,1);
  //if( result!=aoresult_ok ) { Serial.printf("ERROR i2cenable %s\n", aoresult_to_str(result) ); return; }

  // Loop over all nodes
  for( uint16_t addr=1; addr<=last; addr++ ) {
    uint8_t  stat;
    result = aoosp_send_readstat(addr, &stat );
    if( result!=aoresult_ok ) { Serial.printf("ERROR readstat %s\n", aoresult_to_str(result) ); break; }
    uint8_t  com;
    result = aoosp_send_readcomst(addr, &com );
    if( result!=aoresult_ok ) { Serial.printf("ERROR readcomst %s\n", aoresult_to_str(result) ); break; }
    Serial.printf("%4s [%03X ", aoosp_prt_com_sio1(com), addr );
    uint32_t id;
    result = aoosp_send_identify(addr, &id );
    if( result!=aoresult_ok ) { Serial.printf("ERROR identify %s\n", aoresult_to_str(result) ); break; }
    if( AOOSP_IDENTIFY_IS_SAID(id) ) {
      Serial.printf("SAID %s ", com&AOOSP_COMST_DIR_LOOP ? "loop" : "bidir" );
    } else if(  AOOSP_IDENTIFY_IS_RGBI(id) ) {
      Serial.printf("RGBI %s ", stat&AOOSP_STAT_FLAGS_DIRLOOP ? "loop" : "bidir" );
    } else {
      Serial.printf("UNKNOWN ");
    }
    Serial.printf("%s]", aoosp_prt_stat_state(stat) );
    if(  AOOSP_IDENTIFY_IS_SAID(id) ) {
      int enable;
      result= aoosp_exec_i2cenable_get(addr,&enable);
      if( result!=aoresult_ok ) { Serial.printf("ERROR i2cenable_get %s\n", aoresult_to_str(result) ); break; }
      Serial.printf("[RGB][RGB]" );
      if( enable ) Serial.printf("[I2C] " ); else Serial.printf("[RGB] " );
    } else if(  AOOSP_IDENTIFY_IS_RGBI(id) ) {
      Serial.printf("[RGB] " );
    } else {
      Serial.printf("[?] " );
    }
    Serial.printf("%-4s\n", aoosp_prt_com_sio2(com) );
  }

  Serial.printf("\n");
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_topo.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();
  //aoosp_loglevel_set(aoosp_loglevel_tele);
  Serial.printf("\n");

  topo();
}


void loop() {
  delay(5000);
}
