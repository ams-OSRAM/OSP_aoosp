// aoosp_send.cpp - send command telegrams (and receive response telegrams)
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
#ifndef _AOOSP_SEND_H_
#define _AOOSP_SEND_H_


#include <stdint.h>
#include <aoresult.h>


// === LOG ================================================


// When set to 0, the aoosp library has logging enabled for telegram send and receive
#define AOOSP_LOG_ENABLED 1


#if AOOSP_LOG_ENABLED
  // The level for logging aoosp telegrams to Serial
  typedef enum aoosp_loglevel_e {
    aoosp_loglevel_none, // Nothing is logged (default)
    aoosp_loglevel_args, // Logging of sent and received telegram arguments
    aoosp_loglevel_tele, // Also logs raw (sent and received) telegram bytes
  } aoosp_loglevel_t;
  void             aoosp_loglevel_set(aoosp_loglevel_t level);
  aoosp_loglevel_t aoosp_loglevel_get();
#else
  #define aoosp_loglevel_set(level) /* empty */
  #define aoosp_loglevel_get()      /* empty */
#endif // AOOSP_LOG_ENABLED


// === TELEGRAM ADDRESSES =================================


// OSP addresses are 10 bits, some values have a special meaning.
#define AOOSP_ADDR_GLOBALMIN             ( 0x000 )
#define AOOSP_ADDR_GLOBALMAX             ( 0x3FE )

#define AOOSP_ADDR_BROADCAST             ( 0x000 )

#define AOOSP_ADDR_UNICASTMIN            ( 0x001 )
#define AOOSP_ADDR_UNICASTMAX            ( 0x3EF )

#define AOOSP_ADDR_GROUP0                ( 0x3F0 )
#define AOOSP_ADDR_GROUP1                ( 0x3F1 )
#define AOOSP_ADDR_GROUP2                ( 0x3F2 )
#define AOOSP_ADDR_GROUP3                ( 0x3F3 )
#define AOOSP_ADDR_GROUP4                ( 0x3F4 )
#define AOOSP_ADDR_GROUP5                ( 0x3F5 )
#define AOOSP_ADDR_GROUP6                ( 0x3F6 )
#define AOOSP_ADDR_GROUP7                ( 0x3F7 )
#define AOOSP_ADDR_GROUP8                ( 0x3F8 )
#define AOOSP_ADDR_GROUP9                ( 0x3F9 )
#define AOOSP_ADDR_GROUP10               ( 0x3FA )
#define AOOSP_ADDR_GROUP11               ( 0x3FB )
#define AOOSP_ADDR_GROUP12               ( 0x3FC )
#define AOOSP_ADDR_GROUP13               ( 0x3FD )
#define AOOSP_ADDR_GROUP14               ( 0x3FE )
#define AOOSP_ADDR_GROUP(n)              ( (n)<0 || (n)>14 ? AOOSP_ADDR_UNINIT : AOOSP_ADDR_GROUP0+(n) ) // map illegal group n to illegal address

#define AOOSP_ADDR_UNINIT                ( 0x3FF )

#define AOOSP_ADDR_ISBROADCAST(addr)     ( (addr)==AOOSP_ADDR_BROADCAST )
#define AOOSP_ADDR_ISUNICAST(addr)       ( AOOSP_ADDR_UNICASTMIN<=(addr) && (addr)<=AOOSP_ADDR_UNICASTMAX )
#define OAOSP_ADDR_ISMULTICAST(addr)     ( AOOSP_ADDR_GROUP0<=(addr) && (addr)<=AOOSP_ADDR_GROUP14 )
#define AOOSP_ADDR_ISOK(addr)            ( AOOSP_ADDR_ISBROADCAST(addr) || AOOSP_ADDR_ISUNICAST(addr) || OAOSP_ADDR_ISMULTICAST(addr) )


// === TELEGRAMS ==========================================


// This is a list of all telegrams (actually, of functions sending telegrams).
// Several TIDs (telegram IDs) are still reserved in OSP.
// Others have not yet been implemented in this library.


// Telegram 00 RESET - resets all nodes in the chain (all "off"; they also lose their address).
aoresult_t aoosp_send_reset(uint16_t addr );


// Telegram 01 CLRERROR - clears the error flags of the addressed node.
aoresult_t aoosp_send_clrerror(uint16_t addr);


// Telegram 02 INITBIDIR - assigns an address to each node; also configures all nodes for BiDir.
aoresult_t aoosp_send_initbidir(uint16_t addr, uint16_t * last, uint8_t * temp, uint8_t * stat );


// Telegram 03 INITLOOP - assigns an address to each node; also configures all nodes for Loop.
aoresult_t aoosp_send_initloop(uint16_t addr, uint16_t * last, uint8_t * temp, uint8_t * stat );


// Telegram 04 GOSLEEP - switches the state of the addressed node to sleep.
aoresult_t aoosp_send_gosleep(uint16_t addr );


// Telegram 05 GOACTIVE - switches the state of the addressed node to active.
aoresult_t aoosp_send_goactive(uint16_t addr );


// Telegram 06 GODEEPSLEEP switches the state of the addressed node to deep sleep.
/* Not yet implemented */


//  +-------+-------------------+-----------------------+-----------+
//  |3 3 2 2|2 2 2 2 2 2 2 2 1 1|1 1 1 1 1 1 1 1 0 0 0 0|0 0 0 0 0 0|
//  |2 1 9 8|7 6 5 4 3 2 1 0 9 8|7 6 5 4 3 2 1 0 9 8 7 6|5 4 3 2 1 0|
//  +---4---+--------10---------+----------12-----------+-----6-----+
//  |devTYPE|    MANUfacturer   | PART identification   |  REVision |
//  +-------+-------------------+-----------------------+-----------+
#define AOOSP_IDENTIFY_PART_RGBI       0x000
#define AOOSP_IDENTIFY_PART_SAID       0x001
                                       
#define AOOSP_IDENTIFY_MANU_AMSOSRAM   0x000
                                       
#define AOOSP_IDENTIFY_ID2TYPE(id)     (((id)>>28)&0x00F) // Device type
#define AOOSP_IDENTIFY_ID2MANU(id)     (((id)>>18)&0x3FF) // Manufacturer code
#define AOOSP_IDENTIFY_ID2PART(id)     (((id)>> 6)&0xFFF) // Part identification
#define AOOSP_IDENTIFY_ID2REV(id)      (((id)>> 0)&0x03F) // Revision

#define AOOSP_IDENTIFY_MANUPART_RGBI   0x000
#define AOOSP_IDENTIFY_MANUPART_SAID   0x001

#define AOOSP_IDENTIFY_ID2MANUPART(id) (((id)>> 6)&0x3FFFFF) // Manufacturer code

#define AOOSP_IDENTIFY_IS_RGBI(id)     ( AOOSP_IDENTIFY_ID2MANUPART(id)==AOOSP_IDENTIFY_MANUPART_RGBI )
#define AOOSP_IDENTIFY_IS_SAID(id)     ( AOOSP_IDENTIFY_ID2MANUPART(id)==AOOSP_IDENTIFY_MANUPART_SAID )
// Telegram 07 IDENTIFY - asks the addressed node to respond with its ID.
aoresult_t aoosp_send_identify(uint16_t addr, uint32_t * id );


// Telegram 08 P4ERRBIDIR - ping for error in BiDir mode (first device with error answers, otherwise forwards).
/* Not yet implemented */


// Telegram 09 P4ERRLOOP - ping for error in Loop mode (first device with error answers, otherwise forwards).
/* Not yet implemented */


// Telegram 0A ASKTINFO - returns the aggregated max and min temperature across the daisy chain.
/* Not yet implemented */


// Telegram 0B ASKVINFO - returns the aggregated max and min voltage headroom among all drivers the daisy chain.
/* Not yet implemented */


// Telegram 0C READMULT - bits 0 to E in register MULT assigns a node to a group (3F0..3FE). This commands requests MULT.
aoresult_t aoosp_send_readmult(uint16_t addr, uint16_t *groups );


// Telegram 0D SETMULT - assigns the addressed node to zero or more of the 15 groups.
aoresult_t aoosp_send_setmult(uint16_t addr, uint16_t groups);


// Telegram 0E -- free
/* Reserved telegram ID */


// Telegram 0F SYNC - a sync event (via external pin or via this command), activates all drivers with pre-configured settings.
aoresult_t aoosp_send_sync(uint16_t addr);


// Telegram 10 -- free
/* Reserved telegram ID */


// Telegram 11 IDLE - part of OTP write procedure: CUST/FOUNDRY, BURN, IDLE; stops burning process.
aoresult_t aoosp_send_idle(uint16_t addr);


// Telegram 12 FOUNDRY - part of OTP write procedure: CUST/FOUNDRY, BURN, IDLE; selects OTP area reserved for OSP node manufacturer.
aoresult_t aoosp_send_foundry(uint16_t addr);


// Telegram 13 CUST - part of OTP write procedure: CUST/FOUNDRY, BURN, IDLE; selects OTP area reserved for customers.
aoresult_t aoosp_send_cust(uint16_t addr);


// Telegram 14 BURN - part of OTP write procedure: CUST/FOUNDRY, BURN, IDLE; activates the burning from OTP mirror to fuses.
aoresult_t aoosp_send_burn(uint16_t addr);


// Telegram 15 AREAD
/* Not yet implemented */


// Telegram 16 LOAD
/* Not yet implemented */


// Telegram 17 GLOAD
/* Not yet implemented */


// Telegram 18 I2CREAD - requests a SAID to master a read on its I2C bus.
aoresult_t aoosp_send_i2cread8(uint16_t addr, uint8_t daddr7, uint8_t raddr, uint8_t count );


// Telegram 19 I2CWRITE - requests a SAID to master a write on its I2C bus.
aoresult_t aoosp_send_i2cwrite8(uint16_t addr, uint8_t daddr7, uint8_t raddr, const uint8_t * buf, int count);


// Telegram 1A -- free
/* Reserved telegram ID */


// Telegram 1B -- free
/* Reserved telegram ID */


// Telegram 1C -- free
/* Reserved telegram ID */


// Telegram 1D -- free
/* Reserved telegram ID */


// Telegram 1E READLAST - requests a SAID to return the result of the last I2CREAD.
aoresult_t aoosp_send_readlast(uint16_t addr, uint8_t * buf, int size);


// Telegram 1F -- free
/* Reserved telegram ID */


// Telegram 20 -- RESET has no SR
/* Reserved telegram ID */


// Telegram 21 CLRERROR with SR
/* Not yet implemented */


// Telegram 22 -- INIBIDIR has no SR
/* Reserved telegram ID */


// Telegram 23 -- INITLOOP has no SR
/* Reserved telegram ID */


// Telegram 24 GOSLEEP with SR
/* Not yet implemented */


// Telegram 25 GOACTIVE with SR
aoresult_t aoosp_send_goactive_sr(uint16_t addr, uint8_t * temp, uint8_t * stat);


// Telegram 26 GODEEPSLEEP with SR
/* Not yet implemented */


// Telegram 27 -- IDENTIFY has no SR
/* Reserved telegram ID */


// Telegram 28 -- P4ERRBIDIR has no SR
/* Reserved telegram ID */


// Telegram 29 -- P4ERRLOOP has no SR
/* Reserved telegram ID */


// Telegram 2A -- ASK_TINFO has no SR
/* Reserved telegram ID */


// Telegram 2B -- ASK_VINFO has no SR
/* Reserved telegram ID */


// Telegram 2C -- READMULT has no SR
/* Reserved telegram ID */


// Telegram 2D SETMULT with SR
/* Not yet implemented */


// Telegram 2E -- free
/* Reserved telegram ID */


// Telegram 2F -- SYNC has no SR
/* Reserved telegram ID */


// Telegram 30 -- free
/* Reserved telegram ID */


// Telegram 31 IDLE with SR
/* Not yet implemented */


// Telegram 32 FOUNDRY with SR
/* Not yet implemented */


// Telegram 33 CUST with SR
/* Not yet implemented */


// Telegram 34 BURN with SR
/* Not yet implemented */


// Telegram 35 AREAD with SR
/* Not yet implemented */


// Telegram 36 LOAD with SR
/* Not yet implemented */


// Telegram 37 GLOAD with SR
/* Not yet implemented */


// Telegram 38 I2CREAD with SR
/* Not yet implemented */


// Telegram 39 I2CWRITE with SR
/* Not yet implemented */


// Telegram 3A -- free
/* Reserved telegram ID */


// Telegram 3B -- free
/* Reserved telegram ID */


// Telegram 3C -- free
/* Reserved telegram ID */


// Telegram 3D -- free
/* Reserved telegram ID */


// Telegram 3E -- READLAST has no SR
/* Reserved telegram ID */


// Telegram 3F -- free
/* Reserved telegram ID */


#define AOOSP_STAT_FLAGS_OTPCRC1      0x20 // OTP crc error (or in test mode)
#define AOOSP_STAT_FLAGS_TESTMODE     0x20 // (OTP crc error or) in test mode
#define AOOSP_STAT_FLAGS_OV           0x10 // Over Voltage [SAID only]
#define AOOSP_STAT_FLAGS_DIRLOOP      0x10 // DIRection is LOOP (not bidir) [RGBI only]
#define AOOSP_STAT_FLAGS_CE           0x08 // Communication error
#define AOOSP_STAT_FLAGS_LOS          0x04 // Led Open or Short
#define AOOSP_STAT_FLAGS_OT           0x02 // Over Temperature
#define AOOSP_STAT_FLAGS_UV           0x01 // Under Voltage
#define AOOSP_STAT_FLAGS_RGBI_ERRORS  ( AOOSP_STAT_FLAGS_OTPCRC1 |                       AOOSP_STAT_FLAGS_CE | AOOSP_STAT_FLAGS_LOS | AOOSP_STAT_FLAGS_OT | AOOSP_SETUP_FLAGS_UV )
#define AOOSP_STAT_FLAGS_SAID_ERRORS  ( AOOSP_STAT_FLAGS_OTPCRC1 | AOOSP_STAT_FLAGS_OV | AOOSP_STAT_FLAGS_CE | AOOSP_STAT_FLAGS_LOS | AOOSP_STAT_FLAGS_OT | AOOSP_SETUP_FLAGS_UV )
// Telegram 40 READSTAT - asks the addressed node to respond with its (system) status.
aoresult_t aoosp_send_readstat(uint16_t addr, uint8_t * stat);


// Telegram 41 -- no SETSTAT
/* Reserved telegram ID */


// Telegram 42 READTEMPSTAT - asks the addressed node to respond with its temperature and (system) status.
aoresult_t aoosp_send_readtempstat(uint16_t addr, uint8_t * temp, uint8_t * stat );


// Telegram 43 -- no SETTEMPSTAT
/* Reserved telegram ID */


#define AOOSP_COMST_DIR_LOOP         0b10000
#define AOOSP_COMST_DIR_BIDIR        0b00000

#define AOOSP_COMST_SIO1_MASK         0b0011
#define AOOSP_COMST_SIO1_LVDS         0b0000
#define AOOSP_COMST_SIO1_EOL          0b0001
#define AOOSP_COMST_SIO1_MCU          0b0010
#define AOOSP_COMST_SIO1_CAN          0b0011

#define AOOSP_COMST_SIO2_MASK         0b1100
#define AOOSP_COMST_SIO2_LVDS         0b0000
#define AOOSP_COMST_SIO2_EOL          0b0100
#define AOOSP_COMST_SIO2_MCU          0b1000
#define AOOSP_COMST_SIO2_CAN          0b1100
// Telegram 44 READCOMST - asks the addressed node to respond with its communication status 00=LVDS, 01=EOL, 10=MCU, 11=CAN.
aoresult_t aoosp_send_readcomst(uint16_t addr, uint8_t * com );


// Telegram 45 -- no SETCOMST
/* Reserved telegram ID */


// Telegram 46 READLEDST - asks the addressed node to respond LED status (RO red open, GO, BO, RS red short, GS, BS).
/* Not yet implemented */


// Telegram 47 -- no SETLEDST
/* Reserved telegram ID */


// Telegram 48 READTEMP - asks the addressed node to respond with its raw temperature.
aoresult_t aoosp_send_readtemp(uint16_t addr, uint8_t * temp);


// Telegram 49 -- no SETTEMP
/* Reserved telegram ID */


// Telegram 4A READOTTH - asks the addressed node to respond with Over Temperature ThresHold configuration.
/* Not yet implemented */


// Telegram 4B SETOTTH - configures the Over Temperature ThresHold (cycle length, low and high value).
/* Not yet implemented */


#define AOOSP_SETUP_FLAGS_PWMF       0x80 // PWM uses Fast clock
#define AOOSP_SETUP_FLAGS_COMCLKINV  0x40 // mcu spi COMmunication has CLocK INVerted
#define AOOSP_SETUP_FLAGS_CRCEN      0x20 // OTP crc error or in test mode
#define AOOSP_SETUP_FLAGS_OTP        0x10 // OTP crc error or in test mode [SAID only]
#define AOOSP_SETUP_FLAGS_TEMPCK     0x10 // TEMPerature sensor has low update ClocK [RGBI only]
#define AOOSP_SETUP_FLAGS_CE         0x08 // Communication Error
#define AOOSP_SETUP_FLAGS_LOS        0x04 // Led Open or Short
#define AOOSP_SETUP_FLAGS_OT         0x02 // Over Temperature
#define AOOSP_SETUP_FLAGS_UV         0x01 // Under Voltage
#define AOOSP_SETUP_FLAGS_RGBI_DFLT  ( AOOSP_SETUP_FLAGS_TEMPCK | AOOSP_SETUP_FLAGS_OT | AOOSP_SETUP_FLAGS_UV )
#define AOOSP_SETUP_FLAGS_SAID_DFLT  ( AOOSP_SETUP_FLAGS_OTP    | AOOSP_SETUP_FLAGS_OT | AOOSP_SETUP_FLAGS_UV )
// Telegram 4C READSETUP - asks the addressed node to respond with its setup (e.g. CRC check enabled).
aoresult_t aoosp_send_readsetup(uint16_t addr, uint8_t *flags );


// Telegram 4D SETSETUP - sets the setup of the addressed node (e.g. CRC check enabled).
aoresult_t aoosp_send_setsetup(uint16_t addr, uint8_t flags );


// Telegram 4E (variant 0) READPWM - asks the addressed node to respond with its PWM settings (single channel nodes).
aoresult_t aoosp_send_readpwm(uint16_t addr, uint16_t *red, uint16_t *green, uint16_t *blue, uint8_t *daytimes );


// Telegram 4E (variant 1) READPWMCHN - asks the addressed node to respond with its PWM settings of one of its channel.
aoresult_t aoosp_send_readpwmchn(uint16_t addr, uint8_t chn, uint16_t *red, uint16_t *green, uint16_t *blue );


// Telegram 4F (variant 0) SETPWM - configures the PWM settings of the addressed node (single channel nodes).
aoresult_t aoosp_send_setpwm(uint16_t addr, uint16_t red, uint16_t green, uint16_t blue, uint8_t daytimes );


// Telegram 4F (variant 1) SETPWMCHN - configures the PWM settings of one channel of the addressed node.
aoresult_t aoosp_send_setpwmchn(uint16_t addr, uint8_t chn, uint16_t red, uint16_t green, uint16_t blue );


#define AOOSP_CURCHN_FLAGS_RESRVD  0x08
#define AOOSP_CURCHN_FLAGS_SYNCEN  0x04
#define AOOSP_CURCHN_FLAGS_HYBRID  0x02
#define AOOSP_CURCHN_FLAGS_DITHER  0x01
#define AOOSP_CURCHN_FLAGS_DEFAULT 0x00
#define AOOSP_CURCHN_CUR_DEFAULT   0x00
// cur   0    1    2    3    4
// chn0  3mA  6mA 12mA 24mA 48mA
// chn1 1.5mA 3mA  6mA 12mA 24mA
// chn2 1.5mA 3mA  6mA 12mA 24mA
// Telegram 50 READCURCHN - asks the addressed node to respond with the current levels of the specified channel.
aoresult_t aoosp_send_readcurchn(uint16_t addr, uint8_t chn, uint8_t *flags, uint8_t *rcur, uint8_t *gcur, uint8_t *bcur );


// Telegram 51 SETCURCHN - configures the current levels of the addressed node for the specified channel.
aoresult_t aoosp_send_setcurchn(uint16_t addr, uint8_t chn, uint8_t flags, uint8_t rcur, uint8_t gcur, uint8_t bcur);


// Telegram 52 READTCOEFF - asks the addressed node to respond with the temperature coefficients of the requested channel.
/* Not yet implemented */


// Telegram 53 SETTCOEFF - sets RGB temperature coefficients (and a reference) of a specified channel.
/* Not yet implemented */


// Telegram 54 READADC - asks the addressed node to respond with the ADC configuration (the data is in reg 5C).
/* Not yet implemented */


// Telegram 55 SETADC  - configures the ADC to measure Vf of the 3x3 LED.
/* Not yet implemented */


#define AOOSP_I2CCFG_FLAGS_INT      0x08 // Status of INT pin
#define AOOSP_I2CCFG_FLAGS_12BIT    0x04 // Uses 12 bit addressing mode
#define AOOSP_I2CCFG_FLAGS_NACK     0x02 // Last i2c transaction ended with NACK
#define AOOSP_I2CCFG_FLAGS_BUSY     0x01 // Last i2c transaction still busy
#define AOOSP_I2CCFG_FLAGS_DEFAULT  0x00 // Hardware default in SAID

#define AOOSP_I2CCFG_SPEED_1000kHz  0x01 // Fast-mode Plus (Fm+) nominal frequency for readability
#define AOOSP_I2CCFG_SPEED_874kHz   0x01
#define AOOSP_I2CCFG_SPEED_506kHz   0x02 
#define AOOSP_I2CCFG_SPEED_400kHz   0x03 // Fast-mode (Fm) nominal frequency for readability
#define AOOSP_I2CCFG_SPEED_356kHz   0x03
#define AOOSP_I2CCFG_SPEED_275kHz   0x04 
#define AOOSP_I2CCFG_SPEED_224kHz   0x05 
#define AOOSP_I2CCFG_SPEED_189kHz   0x06 
#define AOOSP_I2CCFG_SPEED_163kHz   0x07 
#define AOOSP_I2CCFG_SPEED_144kHz   0x08 
#define AOOSP_I2CCFG_SPEED_128kHz   0x09 
#define AOOSP_I2CCFG_SPEED_116kHz   0x0A 
#define AOOSP_I2CCFG_SPEED_106kHz   0x0B 
#define AOOSP_I2CCFG_SPEED_100kHz   0x0C // Standard-mode (Sm) nominal frequency for readability (default)
#define AOOSP_I2CCFG_SPEED_97kHz    0x0C
#define AOOSP_I2CCFG_SPEED_90kHz    0x0D
#define AOOSP_I2CCFG_SPEED_84kHz    0x0E
#define AOOSP_I2CCFG_SPEED_78kHz    0x0F
#define AOOSP_I2CCFG_SPEED_DEFAULT  0x0C // Hardware default in SAID
// Telegram 56 READI2CCFG - asks the addressed node to respond with the I2C configuration/status.
aoresult_t aoosp_send_readi2ccfg(uint16_t addr, uint8_t *flags, uint8_t * speed );


// Telegram 57 SETI2CCFG - sets the I2C configuration/status of the addressed node.
aoresult_t aoosp_send_seti2ccfg(uint16_t addr, uint8_t flags, uint8_t speed );


// Telegram 58 READOTP - asks the addressed node to respond with content bytes from OTP memory.
aoresult_t aoosp_send_readotp(uint16_t addr, uint8_t otpaddr, uint8_t * buf, int size);


// Telegram 59 SETOTP - writes bytes to the OTP memory of the addressed node.
aoresult_t aoosp_send_setotp(uint16_t addr, uint8_t otpaddr, uint8_t * buf, int size);


// Telegram 5A READTESTDATA asks the addressed node to respond with its test register.
/* Not yet implemented */


// Telegram 5B SETTESTDATA - sets the test register of the addressed node.
aoresult_t aoosp_send_settestdata(uint16_t addr, uint16_t data );


// Telegram 5C READADCDAT - returns ADC value configured via reg 55.
/* Not yet implemented */


// Telegram 5D TESTSCAN
/* Not yet implemented */


// Telegram 5E -- no GETTESTPW
/* Reserved telegram ID */


#define AOOSP_SAID_TESTPW_UNKNOWN 0x0000FFffFFffFFffULL // Use this as SAID password if the password is unknown - it gives warnings 
// Telegram 5F SETTESTPW - sets the test password of the addressed node.
aoresult_t aoosp_send_settestpw(uint16_t addr, uint64_t pw);


// Telegram 60 -- READSTAT with SR
/* Not yet implemented */


// Telegram 61 -- no SETSTAT (with SR)
/* Reserved telegram ID */


// Telegram 62 -- READTEMPST with SR
/* Not yet implemented */


// Telegram 63 -- no SETTEMPSTAT (with SR)
/* Reserved telegram ID */


// Telegram 64 -- READCOMST with SR
/* Not yet implemented */


// Telegram 65 -- no SETCOMST (with SR)
/* Reserved telegram ID */


// Telegram 66 -- READLEDST  with SR
/* Not yet implemented */


// Telegram 67 -- no SETLEDST (with SR)
/* Reserved telegram ID */


// Telegram 68 -- READTEMP with SR
/* Not yet implemented */


// Telegram 69 -- no SETTEMP (with SR)
/* Reserved telegram ID */


// Telegram 6A -- no READOTTH with SR
/* Reserved telegram ID */


// Telegram 6B SETOTTH with SR
/* Not yet implemented */


// Telegram 6C -- no READSETUP with SR
/* Reserved telegram ID */


// Telegram 6D SETSETUP with SR
/* Not yet implemented */


// Telegram 6E -- no READPWM with SR
/* Reserved telegram ID */


// Telegram 6F SETPWM/CHN with SR
/* Not yet implemented */


// Telegram 70 -- no READCURCHN with SR
/* Reserved telegram ID */


// Telegram 71 SETCURCHN with SR
/* Not yet implemented */


// Telegram 72 -- no READTCOEFF with SR
/* Reserved telegram ID */


// Telegram 73 SETTCOEFF with SR
/* Not yet implemented */


// Telegram 74 -- no READADC with SR
/* Reserved telegram ID */


// Telegram 75 SETADC with SR
/* Not yet implemented */


// Telegram 76 -- no READI2CCFG with SR
/* Reserved telegram ID */


// Telegram 77 WRITEI2CCFG with SR
/* Not yet implemented */


// Telegram 78 -- no READOTP with SR
/* Reserved telegram ID */


// Telegram 79 SETOTP with SR
/* Not yet implemented */


// Telegram 7A -- no READTESTDATA with SR
/* Reserved telegram ID */


// Telegram 7B -- no READTESTDATA with SR
/* Reserved telegram ID */


// Telegram 7C -- no SETADCDAT with SR
/* Reserved telegram ID */


// Telegram 7D -- no TESTSCAN with SR
/* Reserved telegram ID */


// Telegram 7E -- no READTESTPW with SR
/* Reserved telegram ID */


// Telegram 7F SETTESTPW with SR
/* Not yet implemented */


#endif

