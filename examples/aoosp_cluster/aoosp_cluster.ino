// aoosp_cluster.ino - illustrates how to use driver cluster 
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
This demo shows how to use the clustering feature of the SAID. Clustering 
allows a SAID to supply higher currents to LEDs by clustering (tie together) 
drivers. 

The designer picks a set of (high current) LEDs and connects each LED to 
multiple drivers of the SAID; this adds their currents. For example, 
channel 0 red and channel 0 green could both be connected to one white 
LED which is then driven by 48 + 48 = 96 mA.

The designer also needs to configures the SAID to have drivers that are 
tied to the same LED (on the PCB), run synchronously. This is done with the 
CH_CLUSTERING bits in OTP. Note that there are only 8 cluster configurations, 
and the PCB wiring (which drivers are connected to the same LED) must match 
the clustering configured in OTP.

None of the demo boards in the evaluation kit tie together SAID outputs, so 
true clustering cannot be demonstrated with the evaluation kit alone. On all 
eval boards, each output has its own red, green, blue LED (in an RGB triplet).
We treat those as indicator LEDs to see which channels are clustered and 
activated by setpwmchn telegrams.

This demo pick cluster configuration 7 because it is the "wildest" clustering.
The diagram below shows the 9 drivers, eg on the left channel 0 with drivers 
0.R, 0.G, and 0.B. We see which drivers are tied together for configuration 7,
eg 0.R and 0.G. Finally, we see for each cluster which driver is the "main", 
e.g. 0.R, the others are "sub". Only setpwmchn to main are effective.

    |               |               |
    +---+   +-------+---+---+       +---+---+
    |   |   |       |   |   |       |   |   |
  +-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+
  |0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|
  +---+---+---+   +---+---+---+   +---+---+---+

This demo will configure one SAID for cluster 7, and then send it 9 times a
setpwmchn, each time only activating one driver. The indicator LEDs will show
that only writes to "sub" drivers are discarded, and that writes to "main"
drivers are copied to their associated "sub".

One final warning: to configure the cluster a write to OTP is needed,
so the SAID test password must be set. If not, you get these messages
  ERROR: SAID test password not configured
  WARNING: ask ams-OSRAM for TESTPW and see aoosp_said_testpw_get() for how to set it


HARDWARE
Have a cable from OSP32.OUT connector to SAIDbasic.IN and from
SAIDbasic.OUT to OSP32.IN connector. Alternatively, leave out 
the SAIDbasic board and connect OSP32.OUT directly to OSP32.IN.
We prefer to have the SAIDbasic booard in between, because the
SAID IN on the OSP32 board has its RGB triplets in reverse order
complicating the understanding.
In Arduino select board "ESP32S3 Dev Module".


BEHAVIOR
Nine tests are running. SAID with address 002 will show:
  yellow off   off
  off    off   off
  off    off   off
  blue   white off
  off    off   off
  off    off   off
  off    off   white
  off    off   off
  off    off   off


OUTPUT
Welcome to aoosp_cluster.ino
version: result 0.4.4 spi 0.5.6 osp 0.4.4
spi: init
osp: init
From OTP[0D]=00: cluster=0
To   OTP[0D]=E0: cluster=7

    YELLOW           OFF             OFF
     OFF             OFF             OFF
     OFF             OFF             OFF
     BLUE           WHITE            OFF
     OFF             OFF             OFF
     OFF             OFF             OFF
     OFF             OFF            WHITE
     OFF             OFF             OFF
     OFF             OFF             OFF

From OTP[0D]=E0: cluster=7
To   OTP[0D]=00: cluster=0
done
*/


#define ADDR                   0x002     // address of SAID to test 
#define OTPADDR_CH_CLUSTERING  0x0D      // address in OTP that contains field CH_CLUSTERING
#define OTPMASK_CH_CLUSTERING  0x1F      // mask for the field CH_CLUSTERING
#define OTPMASK_CLUSTER(cl)    ((cl)<<5) // or mask for cluster

#define CASE_DELAY1            2000   // number of ms each case is shown
#define CASE_DELAY2            200    // number of ms pause after each delay
#define VERBOSE                0      // detailed print-out of each case


void cluster_case_Rgb_rgb_rgb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 0/*chn*/, 0x0FFF/*red*/, 0x0000/*green*/, 0x0000/*blue*/); 
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("    YELLOW           OFF             OFF\n");
#if VERBOSE  
  Serial.printf(" onM onS off     off off off     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf(" FFF 000 000\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 0/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}
void cluster_case_rGb_rgb_rgb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 0/*chn*/, 0x0000/*red*/, 0x0FFF/*green*/, 0x0000/*blue*/); 
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     OFF             OFF             OFF\n");
#if VERBOSE  
  Serial.printf(" off off off     off off off     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf(" 000 FFF 000\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 0/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}
void cluster_case_rgB_rgb_rgb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 0/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0FFF/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     OFF             OFF             OFF\n");
#if VERBOSE  
  Serial.printf(" off off off     off off off     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf(" 000 000 FFF\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 0/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}


void cluster_case_rgb_Rgb_rgb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 1/*chn*/, 0x0FFF/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     BLUE           WHITE            OFF\n");
#if VERBOSE  
  Serial.printf(" off off onS     onM onS onS     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf("                 FFF 000 000\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 1/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}
void cluster_case_rgb_rGb_rgb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 1/*chn*/, 0x0000/*red*/, 0x0FFF/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     OFF             OFF             OFF\n");
#if VERBOSE  
  Serial.printf(" off off off     off off off     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf("                 000 FFF 000\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 1/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}
void cluster_case_rgb_rgB_rgb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 1/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0FFF/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     OFF             OFF             OFF\n");
#if VERBOSE  
  Serial.printf(" off off off     off off off     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf("                 000 000 FFF\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 1/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}


void cluster_case_rgb_rgb_Rgb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 2/*chn*/, 0x0FFF/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     OFF             OFF            WHITE\n");
#if VERBOSE  
  Serial.printf(" off off off     off off off     onM onS onS\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf("                                 FFF 000 000\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 2/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}
void cluster_case_rgb_rgb_rGb() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 2/*chn*/, 0x0000/*red*/, 0x0FFF/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     OFF             OFF             OFF\n");
#if VERBOSE  
  Serial.printf(" off off off     off off off     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf("                                 000 FFF 000\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 2/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}
void cluster_case_rgb_rgb_rgB() {
  aoresult_t result;
  result= aoosp_send_setpwmchn(ADDR, 2/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0FFF/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  Serial.printf("     OFF             OFF             OFF\n");
#if VERBOSE  
  Serial.printf(" off off off     off off off     off off off\n");
  Serial.printf("  +---+   +-------+---+---+       +---+---+\n");
  Serial.printf("+-M-+-S-+-S-+   +-M-+-S-+-S-+   +-M-+-S-+-S-+\n");
  Serial.printf("|0.R|0.G|0.B|   |1.R|1.G|1.B|   |2.R|2.G|2.B|\n");
  Serial.printf("+---+---+---+   +---+---+---+   +---+---+---+\n");
  Serial.printf("                                 000 000 FFF\n");
  Serial.printf("\n\n");
#endif
  delay(CASE_DELAY1);
  result= aoosp_send_setpwmchn(ADDR, 2/*chn*/, 0x0000/*red*/, 0x0000/*green*/, 0x0000/*blue*/);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  delay(CASE_DELAY2);
}
                                      

// Prints old cluster, switches to new, prints new cluster. Requires OTP write, needs test passowrd
void set_cluster(int newcluster) {
  aoresult_t result;
  uint8_t    val, cluster;

  // Print old cluster configuration
  result= aoosp_send_readotp(ADDR, OTPADDR_CH_CLUSTERING, &val, 1); cluster = val >> 5;
  if( result!=aoresult_ok ) Serial.printf("aoosp_send_readotp %s\n", aoresult_to_str(result) );
  Serial.printf("From OTP[%02X]=%02X: cluster=%d\n",OTPADDR_CH_CLUSTERING, val, cluster);

  // Check test password
  if( aoosp_said_testpw_get()==AOOSP_SAID_TESTPW_UNKNOWN ) Serial.printf("ERROR: SAID test password not configured\n" );

  // Write new cluster to OTP (mirror)
  result= aoosp_exec_setotp(ADDR, OTPADDR_CH_CLUSTERING, OTPMASK_CLUSTER(newcluster), OTPMASK_CH_CLUSTERING); 
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );

  // Print old cluster configuration
  result= aoosp_send_readotp(ADDR, OTPADDR_CH_CLUSTERING, &val, 1); cluster = val >> 5;
  if( result!=aoresult_ok ) Serial.printf("aoosp_send_readotp %s\n", aoresult_to_str(result) );
  Serial.printf("To   OTP[%02X]=%02X: cluster=%d\n",OTPADDR_CH_CLUSTERING, val, cluster);
}

                            
void cluster_demo() {
  aoresult_t result;
  uint32_t   id;

  // Reset and initialize all nodes and
  result= aoosp_exec_resetinit(); 
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );

  // Check node ADDR is a SAID
  result= aoosp_send_identify(ADDR, &id);
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );
  if( ! AOOSP_IDENTIFY_IS_SAID(id) ) Serial.printf("ERROR: targeting node %03X, but this is not a SAID\n", ADDR );

  // Configure cluster 
  // TIP: comment this line out (or set wrong password) to see how the cases drive the 9 LEDs
  set_cluster(7);

  // Clear the error flags of all nodes; SAIDs have the V flag (over-voltage) after reset, preventing them from going active.
  result= aoosp_send_clrerror(0x000); 
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );

  // Switch the state of all nodes to active (allowing to switch on LEDs).
  result= aoosp_send_goactive(0x000); 
  if( result!=aoresult_ok ) Serial.printf("setpwmchn %s\n", aoresult_to_str(result) );

  // Run all 9 cases of switching one driver on
  Serial.printf("\n");
  cluster_case_Rgb_rgb_rgb();
  cluster_case_rGb_rgb_rgb();
  cluster_case_rgB_rgb_rgb();
  cluster_case_rgb_Rgb_rgb();
  cluster_case_rgb_rGb_rgb();
  cluster_case_rgb_rgB_rgb();
  cluster_case_rgb_rgb_Rgb();
  cluster_case_rgb_rgb_rGb();
  cluster_case_rgb_rgb_rgB();
  Serial.printf("\n");

  // Restore cluster
  set_cluster(0);

  Serial.printf("done\n");
}


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWelcome to aoosp_cluster.ino\n");
  Serial.printf("version: result %s spi %s osp %s\n", AORESULT_VERSION, AOSPI_VERSION, AOOSP_VERSION );

  aospi_init();
  aoosp_init();

  //aoosp_said_testpw_set(...); // set password, or add #include in aoosp.cpp
  
  cluster_demo();  
}


void loop() {
  delay(5000);
}
