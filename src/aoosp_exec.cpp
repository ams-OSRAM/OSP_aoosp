// aoosp_exec.cpp - execute high level OSP routines (several telegrams)
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


#include <Arduino.h>    // Serial.printf
#include <aospi.h>      // aospi_dirmux_set_loop
#include <aoosp_send.h> // aoosp_send_reset, aoosp_send_initloop
#include <aoosp.h>      // aoosp_said_testpw_get
#include <aoosp_exec.h> // own API


// Generic telegram field access macros
#define BITS_MASK(n)         ((1<<(n))-1)                            // series of n bits: BITS_MASK(3)=0b111 (max n=31)
#define BITS_SLICE(v,lo,hi)  ( ((v)>>(lo)) & BITS_MASK((hi)-(lo)) )  // takes bits [lo..hi) from v: BITS_SLICE(0b11101011,2,6)=0b1010


// Record the `last` from the previous call to uint16_t aoosp_exec_resetinit()
static uint16_t aoosp_exec_resetinit_last_;


/*!
    @brief  Sends RESET and INIT telegrams, auto detecting BiDir or Loop.
    @param  last
            Output parameter returning the address of
            the last node - that is the chain length.
    @param  loop
            Output parameter returning the communication direction:
            1 iff Loop, 0 iff BiDir.
    @return aoresult_ok          if all ok,
            aoresult_sys_cabling if cable or terminator missing
            or other error code
    @note   Output parameters are undefined when an error is returned.
    @note   `last` and `loop` maybe NULL (avoids caller to allocate variable).
            Node that `last` is also available via aoosp_exec_resetinit_last(),
            and loop is available via aospi_dirmux_is_loop().
    @note   Controls the BiDir/Loop direction mux via aospi_dirmux_set_xxx.
    @note   First tries loop mode: sends RESET, sets dirmux to loop,
            sends INITLOOP and checks if a response telegram is received.
            If so exits with aoresult_ok.
            If no telegram is received , tries BiDir mode: sends RESET,
            sets dirmux to BiDir, sends INITBIDIR and checks if a response
            telegram is received. If so exits with aoresult_ok.
            If no telegram received in this case either, exits with
            aoresult_sys_cabling.
*/
aoresult_t aoosp_exec_resetinit(uint16_t *last, int *loop) {
  aoresult_t result;
  uint16_t   last_;
  uint8_t    temp_;
  uint8_t    stat_;

  // Set "fail" values for output parameters
  aoosp_exec_resetinit_last_= 0;
  if( last ) *last= 0x000;
  if( loop ) *loop= -1;

  // First, try RESET for INITLOOP
  result= aoosp_send_reset(0x000);
  delayMicroseconds(150);
  if( result!=aoresult_ok ) return result;

  // Try INITLOOP (recall to set mux)
  aospi_dirmux_set_loop();
  result= aoosp_send_initloop(0x001,&last_,&temp_,&stat_);
  if( result==aoresult_ok ) {
    aoosp_exec_resetinit_last_= last_;
    if( last ) *last= last_;
    if( loop ) *loop= 1;
    return result;
  }
  if( result!=aoresult_spi_noclock ) return result;

  // Next, try RESET for INITBIDIR
  result= aoosp_send_reset(0x000);
  delayMicroseconds(150);
  if( result!=aoresult_ok ) return result;

  // Try INITBIDIR (recall to set mux)
  aospi_dirmux_set_bidir();
  result= aoosp_send_initbidir(0x001,&last_,&temp_,&stat_);
  if( result==aoresult_ok ) {
    aoosp_exec_resetinit_last_= last_;
    if( last ) *last= last_;
    if( loop ) *loop= 0;
    return result;
  }
  if( result!=aoresult_spi_noclock ) return result;

  return aoresult_sys_cabling;
}


/*!
    @brief  Returns the address of the last node as determined by the
            last call to aoosp_exec_resetinit().
    @return address of the last node (that is, the total number of nodes)
    @note   As a side effect of calling aoosp_exec_resetinit(), the
            address of the last node is recorded and available for later 
            use through this function.
*/
uint16_t aoosp_exec_resetinit_last() {
  return aoosp_exec_resetinit_last_;
}


/*!
    @brief  Reads the entire OTP and prints the details requested in `flags`.
    @param  addr
            The address of the OSP node to dump the OTP for (unicast).
    @param  flags
            Combination of AOOSP_OTPDUMP_XXX
    @return aoresult_ok if all ok, otherwise an error code.
*/
aoresult_t aoosp_exec_otpdump(uint16_t addr, int flags) {
  #define OTPSIZE 0x20
  #define OTPSTEP 8
  aoresult_t result;
  uint8_t    otp[OTPSIZE];

  // Switch logging off temporarily
  aoosp_loglevel_t prev_log_level= aoosp_loglevel_get();
  aoosp_loglevel_set(aoosp_loglevel_none);
  for( int otpaddr=0x00; otpaddr<OTPSIZE; otpaddr+=OTPSTEP ) {
    result= aoosp_send_readotp(addr,otpaddr,otp+otpaddr,OTPSTEP);
    if( result!=aoresult_ok ) break;
  }
  // Switch logging back to previous state
  aoosp_loglevel_set(prev_log_level);
  if( result!=aoresult_ok ) return result;

  if( flags & AOOSP_OTPDUMP_CUSTOMER_HEX ) {
    Serial.printf("otp: 0x%02X:",0x0D);
    for( int otpaddr=0x0D; otpaddr<0x20; otpaddr+=1 ) Serial.printf(" %02X",otp[otpaddr]);
    Serial.printf("\n");
  }

  if( flags & AOOSP_OTPDUMP_CUSTOMER_FIELDS ) {
    Serial.printf("otp: CH_CLUSTERING     0D.7:5 %d\n", BITS_SLICE(otp[0x0D],5,8) );
    Serial.printf("otp: HAPTIC_DRIVER     0D.4   %d\n", BITS_SLICE(otp[0x0D],4,5) );
    Serial.printf("otp: SPI_MODE          0D.3   %d\n", BITS_SLICE(otp[0x0D],3,4) );
    Serial.printf("otp: SYNC_PIN_EN       0D.2   %d\n", BITS_SLICE(otp[0x0D],2,3) );
    Serial.printf("otp: STAR_NET_EN       0D.1   %d\n", BITS_SLICE(otp[0x0D],1,2) );
    Serial.printf("otp: I2C_BRIDGE_EN     0D.0   %d\n", BITS_SLICE(otp[0x0D],0,1) );
    Serial.printf("otp: *STAR_START       0E.7   %d\n", BITS_SLICE(otp[0x0E],7,8) ); // Bit reserved by the OSP32 eval kit to identify the SAID that splits the chain
    Serial.printf("otp: OTP_ADDR_EN       0E.3   %d\n", BITS_SLICE(otp[0x0E],3,4) );
    Serial.printf("otp: STAR_NET_OTP_ADDR 0E.2:0 %d (0x%03X)\n", BITS_SLICE(otp[0x0E],0,3), BITS_SLICE(otp[0x0E],0,3)<<7 );
  }

  return aoresult_ok;
}


// Notes on OTP
// ============
//
// - SETOTP can only write blocks of 7 bytes, no more, no less
// - SETOTP (and READOTP) have the memory payload in big endian, whereas C-byte arrays are in little endian
// - Addresses beyond the OTP size (beyond 0x1F) are ignored for write, and read as 0x00 (so no wrap-around).
//
// - SETOTP doesn't write to OTP, rather it writes to its mirror (in P2RAM).
// - The OTP mirror is initialized (copied) from OTP at startup (POR - Power On Reset).
// - The OTP mirror is non-persistent over power-cycles.
// - The OTP mirror is persistent over RESET telegram.
//
// - SETOTP only works when the correct password is first sent using SETTESTPW.
// - Without the password set, the SETOTP does not update the OTP mirror.
// - With the password set, the SAID is in "authenticated" mode.
// - When SAID is authenticated, the SETOTP does update the OTP mirror, but not (yet) the OTP.
// - When SAID is authenticated not all telegrams can pass it: some get garbled, making it impossible to reach nodes further on.
// - It is advised to leave authenticated mode; set an incorrect password (eg TESTPW with 0) to prevent garbling.
//
// - To actually update the OTP, write all the values that must be burned in the OTP mirror as described above.
// - Then send the CUST telegram, lower voltage, send the BURN telegram, wait ~5 ms, send the IDLE telegram.
// - OTP bits can only be updated to 1, never (back) to 0.
//
// - Some of the configuration bits (like bit SPI_MODE) are only inspected right after POR, so updating mirror has no effect.


/*!
    @brief  Reads the OTP (mirror) and updates location otpaddr
            by and-ing it with `andmask` then or-ing it with `ormask`,
            then writing the value back to the OTP (mirror).
    @param  addr
            The address to send the telegram to (unicast).
    @param  otpaddr
            The address of the OTP memory.
    @param  ormask
            A mask applied ("ored") after andmask.
    @param  andmask
            A mask applied ("anded") first to old value
    @return aoresult_ok if all ok, otherwise an error code.
    @note   This function needs the SAID test password; it obtains it via 
            `aoosp_said_testpw_get()`. Make sure it is correct.
    @note   The OTP mirror allows bits to be changed to 0; but when the OTP
            mirror is burned to OTP, a 1-bit in OTP stays at 1.
            So it might be wise to keep andmask to 0xFF.
    @note   The write is not to the OTP, but to the OTP mirror in device RAM.
            The mirror is initialized with the OTP content on power on reset.
            The mirror is not re-initialized by a RESET telegram.
*/
aoresult_t aoosp_exec_setotp(uint16_t addr, uint8_t otpaddr, uint8_t ormask, uint8_t andmask) {
  aoresult_t  result;
  if( otpaddr<0x0D || otpaddr>0x1F ) return aoresult_osp_arg; // Only allowed to update customer area

  // This code is complicated by resource management (C implementation of try finally).
  // We set the password, but we _MUST_ undo that (otherwise this SAID garbles passing messages).
  // The following flags indicate whether the password needs to unset.
  // That is done in the handler that we goto on error.
  int resource_pw=0;

  // Set password for writing
  result= aoosp_send_settestpw(addr,aoosp_said_testpw_get());
  if( result!=aoresult_ok ) goto free_resources;
  resource_pw = 1; // password is set, flag that we need to unset

  // Read current OTP row (read is always 8 bytes)
  uint8_t buf[8];
  result= aoosp_send_readotp(addr,otpaddr,buf,8);
  if( result!=aoresult_ok ) goto free_resources;

  // Mask in the new value
  buf[0] = (buf[0] & andmask) | ormask;

  // Write back updated row (write is always 7 bytes, only first one changed)
  result=aoosp_send_setotp(addr,otpaddr,buf,7);
  if( result!=aoresult_ok ) goto free_resources;

  // Ended without errors, still need to free resources
  result=aoresult_ok;

  // Clean up by freeing claimed resources (that is, unset password)
free_resources:
  if( resource_pw ) aoosp_send_settestpw(addr,0); // no further error handling here
  return result;
}


/*!
    @brief  Reads the I2C_BRIDGE_EN bit from OTP (mirror).
    @param  addr
            The address to send the telegram to (unicast).
    @param  enable
            Output parameter returning the value of I2C_BRIDGE_EN.
    @return aoresult_ok if all ok, otherwise an error code.
    @note   It might be more convenient to use aoosp_exec_i2cpower() 
            instead of this function. That one also checks that 
            I2C_BRIDGE_EN is set, and if so, powers the I2C bus, which 
            is needed anyhow for i2C operations.
    @note   Wrapper around aoosp_send_readotp for easy access.
*/
aoresult_t aoosp_exec_i2cenable_get(uint16_t addr, int * enable) {
  if( enable==0 ) return aoresult_outargnull;
  // I2C_BRIDGE_EN
  uint8_t otp_addr = 0x0D;
  uint8_t otp_bit  = 0x01;
  // Read current OTP row
  uint8_t buf[8];
  aoresult_t result = aoosp_send_readotp(addr,otp_addr,buf,8);
  if( result!=aoresult_ok ) return result;
  // Check the OTP bit
  *enable = (buf[0] & otp_bit) != 0;
  return aoresult_ok;
}


/*!
    @brief  Writes the I2C_BRIDGE_EN bit to OTP (mirror).
    @param  addr
            The address to send the telegram to (unicast).
    @param  enable
            The new value for I2C_BRIDGE_EN.
    @return aoresult_ok if all ok, otherwise an error code.
    @note   Wrapper around `aoosp_exec_setotp()` for easy access.
            That function needs the SAID test password; it obtains it via 
            `aoosp_said_testpw_get()`. Make sure it is correct.
    @note   The write is not to the OTP, but to the OTP mirror (RAM).
            The mirror is initialized with the OTP content on power on reset.
            The mirror is not re-initialized by a RESET telegram.
    @note   When the OTP bit I2C_BRIDGE_EN is set, a SAID uses channel 2
            as I2C bridge instead of RGB controller.
    @note   In real products this function is not used: the I2C_BRIDGE_EN
            is flashed in the actual OTP at manufacturing time, not in the 
            set in shadow RAM during runtime.
    @note   See aoosp_exec_i2cpower.
*/
aoresult_t aoosp_exec_i2cenable_set(uint16_t addr, int enable) {
  // I2C_BRIDGE_EN
  uint8_t otp_addr = 0x0D;
  uint8_t otp_bit  = 0x01;
  // Compute masks
  uint8_t andmask = enable ? 0xFF    : ~otp_bit;
  uint8_t ormask =  enable ? otp_bit : 0x00    ;
  // Set
  return aoosp_exec_setotp(addr,otp_addr,ormask,andmask);
}


/*!
    @brief  Checks the addressed SAID if its OTP has the I2C bridge feature
            enabled, if so, powers the I2C bus.
    @param  addr
            The address to send the telegram to (unicast).
    @return aoresult_ok              if all ok
            aoresult_sys_id          when not a SAID
            aoresult_dev_noi2cbridge when SAID has no I2C bridge (bit in OTP)
            other                    telegram error
    @note   Sets highest power for I2C bus (channel 2).
            This is safe, but could be lowered to minimize power consumption,
            depending on RC constant given by pull-up and line capacitance.
*/
aoresult_t aoosp_exec_i2cpower(uint16_t addr) {
  int        enable;
  aoresult_t result;
  uint32_t   id;
  
  // Check (via IDENTITY) if node addr is a SAID
  result = aoosp_send_identify(addr, &id );
  if( result!=aoresult_ok ) return result;
  if( ! AOOSP_IDENTIFY_IS_SAID(id) ) return aoresult_sys_id;

  // Check (via OTP) if I2C bridging is enabled
  result = aoosp_exec_i2cenable_get(addr,&enable);
  if( result!=aoresult_ok ) return result;
  if( !enable ) return aoresult_dev_noi2cbridge;
  
  // Power the bus
  result = aoosp_send_setcurchn(addr, /*chan*/2, /*flags*/0, 4, 4, 4);
  
  // Return result
  return result;
}


/*!
    @brief  Reads the SYNC_PIN_EN bit from OTP (mirror).
    @param  addr
            The address to send the telegram to (unicast).
    @param  enable
            Output parameter returning the value of SYNC_PIN_EN.
    @return aoresult_ok if all ok, otherwise an error code.
    @note   Wrapper around aoosp_send_readotp for easy access.
*/
aoresult_t aoosp_exec_syncpinenable_get(uint16_t addr, int * enable) {
  if( enable==0 ) return aoresult_outargnull;
  // SYNC_PIN_EN
  uint8_t otp_addr = 0x0D;
  uint8_t otp_bit  = 0x04;
  // Read current OTP row
  uint8_t buf[8];
  aoresult_t result = aoosp_send_readotp(addr,otp_addr,buf,8);
  if( result!=aoresult_ok ) return result;
  // Check the OTP bit
  *enable = (buf[0] & otp_bit) != 0;
  return aoresult_ok;
}


/*!
    @brief  Writes the SYNC_PIN_EN bit to OTP (mirror).
    @param  addr
            The address to send the telegram to (unicast).
    @param  enable
            The new value for SYNC_PIN_EN.
    @return aoresult_ok if all ok, otherwise an error code.
    @note   Wrapper around `aoosp_exec_setotp()` for easy access.
            That function needs the SAID test password; it obtains it via 
            `aoosp_said_testpw_get()`. Make sure it is correct.
    @note   The write is not to the OTP, but to the OTP mirror in device RAM.
            The mirror is initialized with the OTP content on power on reset.
            The mirror is not re-initialized by a RESET telegram.
    @note   When the OTP bit SYNC_PIN_EN is set, a SAID uses B1 (the
            channel 1 blue driver) as input for a SYNC trigger (instead
            of using a sync telegram).
*/
aoresult_t aoosp_exec_syncpinenable_set(uint16_t addr, int enable) {
  // SYNC_PIN_EN
  uint8_t otp_addr = 0x0D;
  uint8_t otp_bit  = 0x04;
  // Compute masks
  uint8_t andmask = enable ? 0xFF    : ~otp_bit;
  uint8_t ormask =  enable ? otp_bit : 0x00    ;
  // Set
  return aoosp_exec_setotp(addr,otp_addr,ormask,andmask);
}


/*!
    @brief  Writes `count` bytes from `buf`, into register `raddr` in I2C
            device `daddr7`, attached to OSP node `addr`.
    @param  addr
            The address to send the telegram to (unicast).
    @param  daddr7
            The 7 bits I2C device address used in mastering the write.
    @param  raddr
            The 8 bits register address; the target of the write.
    @param  buf
            Pointer to buffer containing the bytes to send to the I2C device.
    @param  count
            The number of bytes to write from the buffer (1, 2, 4, or 6).
    @return aoresult_ok if all ok, otherwise an error code.
    @note   See aoosp_exec_i2cpower.
    @note   The current implementation only supports the 8 bit mode.
    @note   This issues an I2C transaction consisting of one segment:
            START daddr7+w raddr buf[0] buf[1] .. buf[count-1] STOP
*/
aoresult_t aoosp_exec_i2cwrite8(uint16_t addr, uint8_t daddr7, uint8_t raddr, const uint8_t *buf, uint8_t count) {
  // Send an I2C write telegram
  aoresult_t result = aoosp_send_i2cwrite8(addr,daddr7,raddr,buf,count);
  if( result!=aoresult_ok ) return result;
  // Wait (with timeout) until I2C transaction is completed (not busy)
  uint8_t flags=AOOSP_I2CCFG_FLAGS_BUSY;
  uint8_t tries=10; // 10x 1ms
  while( (flags&AOOSP_I2CCFG_FLAGS_BUSY) && (tries>0) ) {
    uint8_t speed;
    result = aoosp_send_readi2ccfg(addr,&flags,&speed);
    if( result!=aoresult_ok ) return result;
    delay(1);
    tries--;
  }
  // Was transaction successful
  if( flags & AOOSP_I2CCFG_FLAGS_BUSY ) return aoresult_dev_i2ctimeout;
  if( flags & AOOSP_I2CCFG_FLAGS_NACK ) return aoresult_dev_i2cnack;
  return aoresult_ok;
}


/*!
    @brief  Reads `count` bytes into `buf`, from register `raddr` in I2C
            device `daddr7`, attached to OSP node `addr`.
    @param  addr
            The address to send the telegram to (unicast).
    @param  daddr7
            The 7 bits I2C device address used in mastering the write/read.
    @param  raddr
            The 8 bits register address; the target of the read.
    @param  buf
            Pointer to buffer containing the bytes to send to the I2C device.
    @param  count
            The number of bytes to read (1..8).
    @return aoresult_ok if all ok, otherwise an error code.
    @note   See aoosp_exec_i2cpower.
    @note   The current implementation only supports the 8 bit mode.
    @note   This issues an I2C transaction consisting of two segment:
            START daddr7+w raddr START daddr7+r buf[0] buf[1] .. buf[count-1] STOP
*/
// Reads count bytes into buf, from register raddr in i2c device daddr7, attached to node addr.
aoresult_t aoosp_exec_i2cread8(uint16_t addr, uint8_t daddr7, uint8_t raddr, uint8_t *buf, uint8_t count) {
  // Send an I2C read telegram
  aoresult_t result = aoosp_send_i2cread8(addr,daddr7,raddr,count);
  if( result!=aoresult_ok ) return result;
  // Wait (with timeout) until I2C transaction is completed (not busy)
  uint8_t flags=AOOSP_I2CCFG_FLAGS_BUSY;
  uint8_t tries=10; // 10x 1ms
  while( (flags&AOOSP_I2CCFG_FLAGS_BUSY) && (tries>0) ) {
    uint8_t speed;
    result = aoosp_send_readi2ccfg(addr,&flags,&speed);
    if( result!=aoresult_ok ) return result;
    delay(1);
    tries--;
  }
  // Was transaction successful
  if( flags & AOOSP_I2CCFG_FLAGS_BUSY ) return aoresult_dev_i2ctimeout;
  if( flags & AOOSP_I2CCFG_FLAGS_NACK ) return aoresult_dev_i2cnack;
  // Get the read bytes
  result = aoosp_send_readlast(addr,buf,count);
  return result;
}
