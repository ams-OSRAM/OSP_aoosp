// aoosp_exec.h - execute high level OSP routines (several telegrams)
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
#ifndef _AOOSP_EXEC_H_
#define _AOOSP_EXEC_H_


// Sends RESET and INIT telegrams, auto detecting BiDir or Loop.
aoresult_t aoosp_exec_resetinit(uint16_t *last=0, int *loop=0);
// Returns the address of the last node as determined by the last call to aoosp_exec_resetinit().
uint16_t aoosp_exec_resetinit_last();

// Reads the entire OTP and prints details requested in AOOSP_OTPDUMP_XXX flags.
aoresult_t aoosp_exec_otpdump(uint16_t addr, int flags );
// Updates one byte in OTP.
aoresult_t aoosp_exec_setotp(uint16_t addr, uint8_t otpaddr, uint8_t ormask, uint8_t andmask=0xFF);

// Reads the I2C_BRIDGE_EN bit in OTP (mirror).
aoresult_t aoosp_exec_i2cenable_get(uint16_t addr, int * enable);
// Writes the I2C_BRIDGE_EN bit in OTP (mirror). Should not be needed.
aoresult_t aoosp_exec_i2cenable_set(uint16_t addr, int enable);
// Checks if the SAID has an I2C bridge, if so, powers the I2C bus.
aoresult_t aoosp_exec_i2cpower(uint16_t addr);

// Reads the SYNC_PIN_EN bit from OTP (mirror).
aoresult_t aoosp_exec_syncpinenable_get(uint16_t addr, int * enable);
// Writes the SYNC_PIN_EN bit to OTP (mirror).
aoresult_t aoosp_exec_syncpinenable_set(uint16_t addr, int enable);

// Writes to an I2C device connected to a SAID with I2C bridge..
aoresult_t aoosp_exec_i2cwrite8(uint16_t addr, uint8_t daddr7, uint8_t raddr, const uint8_t *buf, uint8_t count);
// Reads from an I2C device connected to a SAID with I2C bridge.
aoresult_t aoosp_exec_i2cread8(uint16_t addr, uint8_t daddr7, uint8_t raddr, uint8_t *buf, uint8_t count);


// flags for aoosp_exec_otpdump determining what to print
#define AOOSP_OTPDUMP_CUSTOMER_HEX    0x01
#define AOOSP_OTPDUMP_CUSTOMER_FIELDS 0x02
#define AOOSP_OTPDUMP_CUSTOMER_ALL    (AOOSP_OTPDUMP_CUSTOMER_HEX | AOOSP_OTPDUMP_CUSTOMER_FIELDS  )

#define AOOSP_OTPADDR_CUSTOMER_MIN    0x0D
#define AOOSP_OTPADDR_CUSTOMER_MAX    0x20


#endif

