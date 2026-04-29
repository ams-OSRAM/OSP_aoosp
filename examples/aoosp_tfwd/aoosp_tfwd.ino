// aoosp_tfwd.ino - measuring round trip time of command-responses
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
This demo measures, in a BiDir chain, the round trip time for telegrams. 
We look at READSTAT (5 byte responses) and READTEMPSTAT (6 byte responses). 
Both telegrams are sent to each node in chain of length 25
(and then 100 times to allow some averaging), we wait for the response and
measure the round trip time t_trip. This is used to analyze telegram timing.
See also aospi_time.ino.

HARDWARE
The demo runs on the OSP32 board. It expects the RGBIstrip and the SAIDlooker
board to be connected to the OUT and IN connector of OSP32. Even though the 
chain loops back, this firmware configures the chain in bidir mode.
As a result we have a chain of 1+20+3+1 or 25 nodes.
In Arduino select board "ESP32S3 Dev Module".

BEHAVIOR
Nothing to be seen, firmware only retrieves timing.

OUTPUT
Welcome to aoosp_tfwd.ino
version: result 0.5.0 spi 1.0.1 osp 0.9.0
spi: init(MCU-B)
osp: init

                          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+----------+
                  address |   001 |   002 |   003 |   004 |   005 |   006 |   007 |   008 |   009 |   00a |   00b |   00c |   00d |   00e |   00f |   010 |   011 |   012 |   013 |   014 |   015 |   016 |   017 |   018 |   019 | average  |
                node type |  SAID |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  RGBI |  SAID |  SAID |  SAID |  SAID |          |
                          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+----------+
          readstat t_trip |  35.1 |  45.5 |  60.2 |  75.9 |  91.0 | 106.7 | 122.4 | 138.2 | 152.1 | 167.4 | 182.8 | 198.3 | 213.6 | 228.7 | 243.7 | 259.1 | 274.6 | 289.9 | 304.5 | 319.8 | 334.8 | 353.8 | 368.4 | 384.1 | 398.5 |          |
      readtempstat t_trip |  38.2 |  48.9 |  63.1 |  78.6 |  94.0 | 109.7 | 125.5 | 140.9 | 154.9 | 170.3 | 185.6 | 201.4 | 216.4 | 231.8 | 246.5 | 262.0 | 277.5 | 292.8 | 307.5 | 322.8 | 337.7 | 356.7 | 371.5 | 388.8 | 401.6 |          |
                          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+----------+
        readstat-readstat |       |  10.3 |  14.7 |  15.7 |  15.2 |  15.7 |  15.6 |  15.8 |  13.9 |  15.3 |  15.4 |  15.5 |  15.3 |  15.1 |  15.0 |  15.3 |  15.6 |  15.3 |  14.6 |  15.3 |  15.0 |  19.0 |  14.6 |  15.7 |  14.4 |  15.1 us |
readtempstat-readtempstat |       |  10.7 |  14.2 |  15.5 |  15.4 |  15.7 |  15.8 |  15.3 |  14.0 |  15.5 |  15.2 |  15.8 |  15.1 |  15.3 |  14.7 |  15.5 |  15.5 |  15.3 |  14.7 |  15.3 |  15.0 |  18.9 |  14.9 |  17.3 |  12.8 |  15.1 us |
                          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+----------+
    readtempstat-readstat |   3.1 |   3.4 |   2.9 |   2.7 |   3.0 |   3.0 |   3.2 |   2.7 |   2.8 |   2.9 |   2.8 |   3.0 |   2.8 |   3.1 |   2.8 |   2.9 |   2.8 |   2.8 |   3.0 |   3.0 |   3.0 |   2.9 |   3.1 |   4.7 |   3.0 |   3.0 us |
                    t_bit |  0.38 |  0.43 |  0.36 |  0.34 |  0.37 |  0.38 |  0.39 |  0.34 |  0.35 |  0.36 |  0.35 |  0.38 |  0.35 |  0.39 |  0.35 |  0.37 |  0.35 |  0.35 |  0.37 |  0.37 |  0.37 |  0.36 |  0.39 |  0.59 |  0.38 |  0.38 us |
                          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+----------+
           readstat t_fwd |       |   6.7 |   7.0 |   7.3 |   7.4 |   7.5 |   7.5 |   7.6 |   7.5 |   7.5 |   7.5 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.7 |   7.6 |   7.7 |   7.6 |   7.5 us |
       readtempstat t_fwd |       |   6.9 |   7.0 |   7.2 |   7.4 |   7.5 |   7.5 |   7.6 |   7.5 |   7.5 |   7.5 |   7.6 |   7.6 |   7.6 |   7.5 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.6 |   7.7 |   7.6 |   7.7 |   7.6 |   7.5 us |
                          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+----------+

ANALYSIS
The first row ("address") lists the 25 (macro LAST) node addresses.
The second row list the node types found with IDENTIFY.
The next two rows are the round trip times (t_trip) using READSTAT
respectively READTEMPSTAT. Note that the numbers are an average over 
100 telegrams (macro REPEAT). Then starts some analysis.

Quoting the API documentation of aospi_txrx_us()
  The trip time (t_trip) includes sending the telegram sized txsize 
  (t_cmd) and receiving the response sized rxsize (t_resp), but 
  also includes the execution time of the command (t_exec) and the 
  forwarding time (t_fwd) of all (k) intermediate nodes . 
  In some cases the executing node introduces a delay (t_delay), 
  typically 5us for SAID in BiDir, 0 otherwise.
  For a BiDir trip to node k+1 the trip time is as follows:
    t_trip = k×t_fwd + t_cmd + t_exec + t_delay + t_resp + k×t_fwd

Row "readstat-readstat" looks at the difference between t_trip for READSTAT 
to node with address n+1 and t_trip for READSTAT to node with address n. 
Same telegram, one node further. The difference is 2×t_fwd. We find that 
2×t_fwd = 15.1 µs and reasonably constant over n. 
Row "readtempstat-readtempstat" does the same for READTEMPSTAT, we 
also find 2×t_fwd = 15.1 µs.

Row "readtempstat-readstat" looks at the difference between t_trip for 
READTEMPSTAT to node n and READSTAT for node n. Different telegram, same node. 
That difference is the difference in the two t_resp's, which comes from the 
difference in reponse telegram length. The length differ 1 byte or 8 bits. 
With the 2.4MHz OSP clock, 8 bits is 3.3 µs. We measure 3.0 µs, 10% off.
The limited resolution of the timer (1 µs) with respect to the small (3 µs)
difference probably contributes to the error. The next row "t_bit" derives 
the bit time from that (dividing by 8) and thus also comes 10% short: 
0.38 µs, where we expect 1/2.4 = 0.42 µs.

The next two rows compute the forwarding time. t_exec is assumed 0,
t_delay is assumed 5, t_cmd and t_resp are computed from t_bit.
  2×k×t_fwd = t_trip - ( t_cmd + t_exec + t_delay + t_resp )
We find an average t_fwd = 7.5 µs for both telegrams.
*/


// === MEASURE ==============================================================


// Measurement data, over LAST nodes
#define LAST 25
// index is node address, starting at 0x001
double readstat_us[LAST+1]; 
double readtempstat_us[LAST+1]; 


// Simple `result` checking
#define CHECK() do { if(result!=0) Serial.printf("error %d/%s\n",result,aoresult_to_str(result)); } while(0)


// Function type to pass to repeat()
typedef void (*func_t)(uint16_t addr);


// readstat in func_t shape
void aopriv_readstat(uint16_t addr) {
  // Serial.printf("readstat(%03x)\n",addr);
  uint8_t stat;
  aoresult_t result= aoosp_send_readstat(addr,&stat); CHECK(); 
}


// readtempstat in func_t shape
void aopriv_readtempstat(uint16_t addr) {
  // Serial.printf("readstat(%03x)\n",addr);
  uint8_t temp; uint8_t stat;
  aoresult_t result= aoosp_send_readtempstat(addr,&temp,&stat); CHECK(); 
}


// Repeat send&receive to node addr with telegram func()
#define REPEAT 100
double repeat( uint16_t addr, func_t func) {
  uint32_t sum=0;
  for(int i=0; i<REPEAT; i++) {
    func(addr);
    sum+= aospi_txrx_us();
  }
  double average= (double)sum/REPEAT;
  return average;
}


void measure_readstat_t_trip() {
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    readstat_us[addr]= repeat( addr, aopriv_readstat );
  }
}


void measure_readtempstat_t_trip() {
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    readtempstat_us[addr]= repeat( addr, aopriv_readtempstat );
  }
}



// === PRINT ================================================================


void print_line() {
  Serial.printf("%25s +","");
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    Serial.printf("-------+");
  }
  Serial.printf("----------+\n");
}


void print_addr() {
  Serial.printf("%25s |","address");
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    Serial.printf("   %03x |",addr);
  }
  Serial.printf(" average  |\n");
}


void print_nodetype() {
  Serial.printf("%25s |","node type");
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    uint32_t id;
    aoresult_t result= aoosp_send_identify(addr,&id); CHECK(); 
    if(      AOOSP_IDENTIFY_IS_RGBI(id)) Serial.printf(" %5s |","RGBI");
    else if( AOOSP_IDENTIFY_IS_SAID(id)) Serial.printf(" %5s |","SAID");
    else                                 Serial.printf(" %5s |","??");
  }
  Serial.printf("          |\n");
}


void print_readstat_t_trip() {
  Serial.printf("%25s |","readstat t_trip");
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    Serial.printf(" %5.1f |",readstat_us[addr]);
  }
  Serial.printf("          |\n");
}


void print_readtempstat_t_trip() {
  Serial.printf("%25s |","readtempstat t_trip");
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    Serial.printf(" %5.1f |",readtempstat_us[addr]);
  }
  Serial.printf("          |\n");
}


void print_readstat_readstat_delta() {
  Serial.printf("%25s |","readstat-readstat");
  Serial.printf(" %5s |","");
  double average_delta_us= 0.0;
  for( uint16_t addr=2; addr<=LAST; addr++ ) {
    double delta= readstat_us[addr]-readstat_us[addr-1];
    average_delta_us += delta;
    Serial.printf(" %5.1f |",delta);
  }
  average_delta_us /= (LAST-1);
  Serial.printf(" %5.1f us |\n", average_delta_us);
}


void print_readtempstat_readtempstat_delta() {
  Serial.printf("%25s |","readtempstat-readtempstat");
  Serial.printf(" %5s |","");
  double average_delta_us= 0.0;
  for( uint16_t addr=2; addr<=LAST; addr++ ) {
    double delta= readtempstat_us[addr]-readtempstat_us[addr-1];
    average_delta_us += delta;
    Serial.printf(" %5.1f |",delta);
  }
  average_delta_us /= (LAST-1);
  Serial.printf(" %5.1f us |\n", average_delta_us);
}


void print_readtempstat_readstat_delta() {
  Serial.printf("%25s |","readtempstat-readstat");
  double average_delta_us= 0.0;
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    double delta= readtempstat_us[addr]-readstat_us[addr];
    average_delta_us += delta;
    Serial.printf(" %5.1f |",delta);
  }
  average_delta_us /= LAST;
  Serial.printf(" %5.1f us |\n", average_delta_us);
}


double average_t_bit_us;


void print_t_bit() {
  Serial.printf("%25s |","t_bit");
  average_t_bit_us= 0.0;
  for( uint16_t addr=1; addr<=LAST; addr++ ) {
    // t_cmd, t_exec, t_delay, t_fwd is assumed to be the same for both readtempstat and readstat
    int bytes_resp_readtempstat = 3+2+1;
    int bytes_resp_readstat = 3+1+1;
    int bytes = bytes_resp_readtempstat - bytes_resp_readstat;
    double t_bit_us= (readtempstat_us[addr]-readstat_us[addr])/(8*bytes);
    average_t_bit_us+= t_bit_us;
    Serial.printf(" %5.2f |",t_bit_us);
  }
  average_t_bit_us /= LAST;
  Serial.printf(" %5.2f us |\n", average_t_bit_us);
}



void print_readstat_t_fwd() {
  Serial.printf("%25s |","readstat t_fwd");
  Serial.printf(" %5s |","");
  double average_t_fwd_us = 0.0;
  for( uint16_t addr=2; addr<=LAST; addr++ ) {
    double t_cmd_us = (3+1)*8*average_t_bit_us;    // header, crc
    double t_resp_us = (3+1+1)*8*average_t_bit_us; // header, stat, crc
    double t_exec_us = 0;  // readstat
    double t_delay_us = 5; // bidir
    double t_fwd_tot_us= readstat_us[addr] - t_cmd_us - t_exec_us - t_delay_us - t_resp_us;
    double t_fwd_us= t_fwd_tot_us / (addr-1) / 2;
    average_t_fwd_us += t_fwd_us;
    Serial.printf(" %5.1f |",t_fwd_us);
  }
  average_t_fwd_us /= (LAST-1);
  Serial.printf(" %5.1f us |\n", average_t_fwd_us);
}



void print_readtempstat_t_fwd() {
  Serial.printf("%25s |","readtempstat t_fwd");
  Serial.printf(" %5s |","");
  double average_t_fwd_us = 0.0;
  for( uint16_t addr=2; addr<=LAST; addr++ ) {
    double t_cmd_us = (3+1)*8*average_t_bit_us;    // header, crc
    double t_resp_us = (3+2+1)*8*average_t_bit_us; // header, temp, stat, crc
    double t_exec_us = 0;  // readtempstat
    double t_delay_us = 5; // bidir
    double t_fwd_tot_us= readtempstat_us[addr] - t_cmd_us - t_exec_us - t_delay_us - t_resp_us;
    double t_fwd_us= t_fwd_tot_us / (addr-1) / 2;
    average_t_fwd_us += t_fwd_us;
    Serial.printf(" %5.1f |",t_fwd_us);
  }
  average_t_fwd_us /= (LAST-1);
  Serial.printf(" %5.1f us |\n", average_t_fwd_us);
}


// === MAIN =================================================================


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_tfwd.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );
  
  aospi_init();
  aoosp_init();

  aospi_dirmux_set_bidir();
  aoresult_t result; uint16_t last; uint8_t temp; uint8_t stat;
  result= aoosp_send_reset(0x000); CHECK(); 
  result= aoosp_send_initbidir(0x001,&last,&temp,&stat); CHECK(); 
  result= aoosp_send_clrerror(0x000); CHECK(); 
  result= aoosp_send_goactive(0x000); CHECK();
  if( last<LAST ) Serial.printf("Chain too short\n");
  Serial.printf("\n");

  measure_readstat_t_trip();
  measure_readtempstat_t_trip();

  print_line();
    print_addr();
    print_nodetype();
  print_line();
    print_readstat_t_trip();
    print_readtempstat_t_trip();
  print_line();
    print_readstat_readstat_delta();
    print_readtempstat_readtempstat_delta();
  print_line();
    print_readtempstat_readstat_delta();
    print_t_bit();
  print_line();
    print_readstat_t_fwd();
    print_readtempstat_t_fwd();
  print_line();
}


void loop() {
}

