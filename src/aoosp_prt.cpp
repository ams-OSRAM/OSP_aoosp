// aoosp_prt.cpp - helpers to pretty print OSP telegrams in human readable form
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


#include <stdio.h>     // snprintf
#include <aoosp_prt.h> // own API


// Tables that map bit fields to human readable strings.
// Note that some are SAID resp RGBi specific
static const char * aoosp_prt_stat_names[]        = { "unintialized", "sleep", "active", "deepsleep" };                // RGBi uses name UNINITIALIZED (SAID uses INITIALIZED but that seems wrong)
static const char * aoosp_prt_stat_flags46_rgbi[] = { "ol", "oL", "Ol", "OL" };                                        // Otp error, bidir/Loop [RGBi only]
static const char * aoosp_prt_stat_flags46_said[] = { "tv", "tV", "Tv", "TV" };                                        // Test mode (or OTP err), over Voltage [SAID only]
static const char * aoosp_prt_stat_flags04[]      = { "clou", "cloU", "clOu", "clOU", "cLou", "cLoU", "cLOu", "cLOU",  // Communication, LED, Over temperature, Under voltage [RGBi and SAID]
                                                      "Clou", "CloU", "ClOu", "ClOU", "CLou", "CLoU", "CLOu", "CLOU",};
static const char * aoosp_prt_setup_flags48[]     = { "pcct", "pccT", "pcCt", "pcCT",  "pcct", "pCcT", "pCCt", "pCCT", // PWM fast, mcu spi CLK inverted, CRC check enabled, Temp sensor slow rate
                                                      "Pcct", "PccT", "PcCt", "pcCT",  "Pcct", "PCcT", "PCCt", "PCCT",};
static const char * aoosp_prt_com_names[]         = { "lvds", "eol","mcu", "can" };
static const char * aoosp_prt_curchn_flags[]      = { "rshd", "rshD", "rsHd", "rsHD", "rShd", "rShD", "rSHd", "rSHD", // Reserved, Sync enabled, Hybrid PWM, Dithering enabled
                                                      "Rshd", "RshD", "RsHd", "RsHD", "RShd", "RShD", "RSHd", "RSHD",};
static const char * aoosp_prt_i2ccfg_flags[]      = { "itnb", "itnB", "itNb", "itNB", "iTnb", "iTnB", "iTNb", "iTNB", // Interrupt, Twelve bit addressing, Nack/ack, I2C transaction Busy
                                                      "itnb", "itnB", "itNb", "itNB", "iTnb", "iTnB", "iTNb", "iTNB",};


// Generic telegram field access macros
#define BITS_MASK(n)                  ((1<<(n))-1)                           // series of n bits: BITS_MASK(3)=0b111 (max n=31)
#define BITS_SLICE(v,lo,hi)           ( ((v)>>(lo)) & BITS_MASK((hi)-(lo)) ) // takes bits [lo..hi) from v: BITS_SLICE(0b11101011,2,6)=0b1010


// "Only use one char* returning pretty print function at a time."
// Pretty print functions that return a char * all use the same global 
// buffer aoosp_prt_str_buf[] for returning the string.
// This means that only one such function can be used at a time.
// For example this will not work: 
//   printf("%s-%s", aoosp_prt_stat_rgbi(x), aoosp_prt_stat_said(x) ).
#define AOOSP_PRT_BUF_SIZE 48
static char aoosp_prt_buf[AOOSP_PRT_BUF_SIZE];



/*!
    @brief  Converts RGBi raw temperature to Celsius.
    @param  temp
            Temperature byte reported by an RGBi OSP node.
    @return The temperature in Celsius.
    @note   For SAID use aoosp_prt_temp_said.
            Value typically comes from READTEMP, READTEMPSTAT or INITxxxx.
    @note   Temperature [°C]= 1.08 x ADC readout value – 126°C
*/
int aoosp_prt_temp_rgbi(uint8_t temp) {
  // scale up with factor 100 for more accurate division in integer domain
  return ((int)(temp)*108+50)/100-126; // +50 for rounding (temp is always positive)
}


/*!
    @brief  Converts SAID raw temperature to Celsius.
    @param  temp
            Temperature byte reported by a SAID OSP node.
    @return The temperature in Celsius.
    @note   For RGBi use aoosp_prt_temp_rgbi.
            Value typically comes from READTEMP, READTEMPSTAT or INITxxxx.
    @note   T(C) = (TEMPVALUE - 116) / 0.85 + 25
*/
int aoosp_prt_temp_said(uint8_t temp) {
  // scale up with factor 100 for more accurate division in integer domain
  int temp100= ((int)(temp)-116)*100; 
  int round= temp100<0 ? -42 : +42; // 42 ~ 85/2
  return (temp100+round)/85+25;
}


/*!
    @brief  Converts a node state to a string.
    @param  stat
            Status byte reported by a node.
    @return One of uninitialized, sleep, active, deepsleep.
            Inspects bit 7 and 6 of stat.
            example "active".
    @note   Value typically comes from READSTAT, READTEMPSTAT or INITxxxx.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_stat_state(uint8_t stat) {
  snprintf( aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s", aoosp_prt_stat_names[BITS_SLICE(stat,6,8)] );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts an RGBi status byte to a string.
    @param  stat
            Status byte reported by a RGBi node.
    @return A string consisting of three parts separated by dashes.
            Part 1 status: uninitialized, sleep, active, deepsleep.
            Part 2 bits 4-5: 1 char for Otp error, bidir/Loop
            Part 3 bits 0-3: 1 char for Communication, LED, Over temperature, Under voltage
            Example "sleep-oL-clou".
    @note   For SAID use aoosp_prt_stat_said.
            Value typically comes from READSTAT, READTEMPSTAT or INITxxxx.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_stat_rgbi(uint8_t stat) {
  snprintf( aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s-%s-%s",
    aoosp_prt_stat_names[BITS_SLICE(stat,6,8)], aoosp_prt_stat_flags46_rgbi[BITS_SLICE(stat,4,6)], aoosp_prt_stat_flags04[BITS_SLICE(stat,0,4)] );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts an SAID status byte to a string.
    @param  stat
            Status byte reported by a SAID node.
    @return A string consisting of three parts separated by dashes.
            Part 1 status: uninitialized, sleep, active, deepsleep.
            Part 2 bits 4-5: Test mode (or otp error), over Voltage
            Part 3 bits 0-3: 1 char for Communication, LED, Over temperature, Under voltage
            Example "active-tv-clou".
    @note   For RGBi use aoosp_prt_stat_rgbi.
            Value typically comes from READSTAT, READTEMPSTAT or INITxxxx.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_stat_said(uint8_t stat) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s-%s-%s",
    aoosp_prt_stat_names[BITS_SLICE(stat,6,8)], aoosp_prt_stat_flags46_said[BITS_SLICE(stat,4,6)], aoosp_prt_stat_flags04[BITS_SLICE(stat,0,4)] );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts a LED status byte to a string.
    @param  ledst
            LED status byte reported by an OSP node.
    @return A string consisting of three parts separated by dashes.
            The parts are the open (O vs o) or short (S vs s) state
            of the red, green and blue driver respectively.
            Example "os-oS-Os".
    @note   Value typically comes from READLEDST or READLEDSTCHN.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_ledst(uint8_t ledst) {
  //  7  6  5  4   3  2  1  0
  // RVS RO GO BO RVS RS GS BS (red, green blue x open short)
  #define oO(pos) ((ledst & (1<<(pos))) ? 'O' : 'o' )
  #define sS(pos) ((ledst & (1<<(pos))) ? 'S' : 's' )
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%c%c-%c%c-%c%c",
    /*red*/ oO(6), sS(2), /*grn*/ oO(5), sS(1), /*blu*/ oO(4), sS(0) );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts an RGBi PWM quartet (from READPWM) to a string.
    @param  red
            The 15 bit PWM setting for the red driver.
    @param  green
            The 15 bit PWM setting for the green driver.
    @param  blue
            The 15 bit PWM setting for the blue driver.
    @param  daytimes
            The 3 bit flags signaling day time (ie high current) 
            for red (MSB), green and blue (LSB) driver.
    @return A string consisting of three parts separated by dashes.
            The three parts are for the red, green and blue and
            have the same rendering: "#.####".
            The # is 0 for night (low current) and 1 for day (high current).
            The #### is hex rendering of the driver value.
            Example "0.0000-1.7FFF-0.0000".
    @note   For SAID use aoosp_prt_pwm_said.
            Value typically comes from READPWM or READPWMCHN.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_pwm_rgbi(uint16_t red, uint16_t green, uint16_t blue, uint8_t daytimes) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%X.%04X-%X.%04X-%X.%04X",
    BITS_SLICE(daytimes,2,3), red, BITS_SLICE(daytimes,1,2), green, BITS_SLICE(daytimes,0,1), blue );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts an SAID PWM triplet (from READPWMCHN) to a string.
    @param  red
            The 16 bit PWM setting for the red driver.
    @param  green
            The 16 bit PWM setting for the green driver.
    @param  blue
            The 16 bit PWM setting for the blue driver.
    @return A string consisting of three parts separated by dashes.
            The three parts are for the red, green and blue and
            have the same rendering: "####" (4 hex digits).
            Example "0000-FFFF-0000".
    @note   For RGBi use aoosp_prt_pwm_rgbi.
            At the moment there is no further detailing 
            of the meaning of the bits
            Value typically comes from READPWM or READPWMCHN.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_pwm_said(uint16_t red, uint16_t green, uint16_t blue) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%04X-%04X-%04X", red, green, blue );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts a communication settings to a string for SIO1.
    @param  com
            The communication setting.
    @return One of (for SIO1) lvds, eol, mcu, can.
            example "lvds".
    @note   Value typically comes from READCOMST.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_com_sio1(uint8_t com) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s", aoosp_prt_com_names[BITS_SLICE(com,0,2)] );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts a communication settings to a string for SIO2.
    @param  com
            The communication setting.
    @return One of (for SIO2) lvds, eol, mcu, can.
            Example "lvds".
    @note   Value typically comes from READCOMST.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_com_sio2(uint8_t com) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s", aoosp_prt_com_names[BITS_SLICE(com,2,4)] );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts an RGBi communication settings to a string.
    @param  com
            The 4 bit communication setting.
    @return A string consisting of two parts separated by a dash.
            The two parts are for SIO1 and SIO2 and 
            have the same rendering: lvds, eol, mcu, can.
            Example "lvds-lvds".
    @note   For SAID use aoosp_prt_com_said.
            Value typically comes from READCOMST.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_com_rgbi(uint8_t com) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s-%s",
    aoosp_prt_com_names[BITS_SLICE(com,2,4)], aoosp_prt_com_names[BITS_SLICE(com,0,2)]  );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts an SAID communication settings to a string.
    @param  com
            The 6 bit communication setting.
    @return A string consisting of three parts separated by dashes.
            The outer two parts are for SIO1 and SIO2 and 
            have the same rendering: lvds, eol, mcu, can.
            The inner part is bidir or loop.
            Example "lvds-loop-lvds".
    @note   For RGBi use aoosp_prt_com_rgbi.
            Value typically comes from READCOMST.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_com_said(uint8_t com) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s-%s-%s",
    aoosp_prt_com_names[BITS_SLICE(com,2,4)], BITS_SLICE(com,4,5)?"loop":"bidir", aoosp_prt_com_names[BITS_SLICE(com,0,2)]  );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts an OSP setup byte to a string.
    @param  flags
            The 8 bit setup byte.
    @return A string consisting of two parts separated by a dash.
            Part 1 bits 4-7: 1 char for PWM fast, mcu spi CLK inverted, CRC check enabled, Temp sensor slow rate.
            Part 2 bits 0-3: 1 char for Communication, LED, Over temperature, Under voltage
            Example "pccT-clOU".
    @note   Value typically comes from READSETUP.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_setup(uint8_t flags) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s-%s",
    aoosp_prt_setup_flags48[BITS_SLICE(flags,4,8)], aoosp_prt_stat_flags04[BITS_SLICE(flags,0,4)]  );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts a byte array (like a telegram) to a string.
    @param  buf
            A pointer to a sequence of bytes.
    @param  size
            Number of bytes of buf to include in the string.
    @return A string consisting 2 hex chars per byte
            Example "A0 09 02 00 50 6D".
    @note   If size is too long, string gets truncated 
            (up to 12 guaranteed to fit).
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_bytes(const void * buf, int size ) {
  char * p = aoosp_prt_buf;
  int remain = AOOSP_PRT_BUF_SIZE;
  for( int i=0; i<size && 3*i<AOOSP_PRT_BUF_SIZE-1; i++ ) {
    int num = snprintf(p, remain, "%02X ", ((uint8_t*)buf)[i] );
    p+=num;
    remain-=num;
  }
  if( size>0 ) *(p-1)='\0'; // overwrite last space
  return aoosp_prt_buf;
}


/*!
    @brief  Converts a channel current setting to a string.
    @param  flags
            The 4 bit channel current setting.
    @return A string consisting of flags; 1 char for 
            Reserved, Sync enabled, Hybrid PWM, Dithering enabled
            Example "rshd".
    @note   Value typically comes from READCUCHN.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_curchn(uint8_t flags) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s", aoosp_prt_curchn_flags[flags] );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts a SAID I2C configuration to a string.
    @param  flags
            The 4 bit I2C configuration .
    @return A string consisting of flags; 1 char for 
            Interrupt, Twelve bit addressing, Nack/ack, I2C transaction Busy
            Example "itnb".
    @note   Value typically comes from READI2CCFG.
    @note   Only use one char* returning pretty print function at a time.
*/
char * aoosp_prt_i2ccfg(uint8_t flags) {
  snprintf(aoosp_prt_buf, AOOSP_PRT_BUF_SIZE, "%s", aoosp_prt_i2ccfg_flags[flags] );
  return aoosp_prt_buf;
}


/*!
    @brief  Converts a SAID I2C bus speed to bits/second.
    @param  speed
            The 4 bit speed setting (1..15, not 0).
    @return The bus speed in bits/second.
            I2C_SPEED  bus freq   kHz
                 0x00    [do not use]
                 0x01    640000   640
                 0x02    417391   417
                 0x03    309677   310  Fast-mode (Fm) 400 kHz
                 0x04    246154   246
                 0x05    204255   204
                 0x06    174545   175
                 0x07    152381   152
                 0x08    135211   135
                 0x09    121519   122
                 0x0A    110345   110
                 0x0B    101053   101
                 0x0C     93204    93  Standard-mode (Sm) 100 kHz (default)
                 0x0D     86486    86
                 0x0E     80672    81
                 0x0F     75591    76
    @note   The `speed` value typically comes from telegram READI2CCFG.
*/
int aoosp_prt_i2ccfg_speed(uint8_t speed) {
  int div = 2*(speed*8+7);
  return ( 19200*1000 + div/2 ) / div;
}


