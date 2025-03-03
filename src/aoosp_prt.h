// aoosp_prt.h - helpers to pretty print OSP telegrams in human readable form
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
#ifndef _AOOSP_PRT_H_
#define _AOOSP_PRT_H_


#include <stdint.h>


// Converts RGBi raw temperature to Celsius.
int    aoosp_prt_temp_rgbi(uint8_t temp);      
// Converts SAID raw temperature to Celsius.
int    aoosp_prt_temp_said(uint8_t temp);      

// Converts a node state to a string (example "ACTIVE").
char * aoosp_prt_stat_state(uint8_t stat);     
// Converts an RGBi status byte to a string (example "SLEEP-oL-clou").
char * aoosp_prt_stat_rgbi(uint8_t stat);      
// Converts an SAID status byte to a string (example "ACTIVE-tv-clou").
char * aoosp_prt_stat_said(uint8_t stat);      
// Converts a LED status byte to a string (eample Example "os-oS-Os").
char * aoosp_prt_ledst(uint8_t ledst);

// Converts an RGBi PWM quartet to a string (example "0.0000/1.7FFF/0.0000").
char * aoosp_prt_pwm_rgbi(uint16_t red, uint16_t green, uint16_t blue, uint8_t daytimes); 
// Converts an SAID PWM triplet to a string (example "0000/FFFF/0000").
char * aoosp_prt_pwm_said(uint16_t red, uint16_t green, uint16_t blue); 

// Converts a communication settings to a string for SIO1 (example "LVDS").
char * aoosp_prt_com_sio1(uint8_t com);        
// Converts a communication settings to a string for SIO2 (example "LVDS").
char * aoosp_prt_com_sio2(uint8_t com);        
// Converts an RGBi communication settings to a string (example "LVDS-LVDS").
char * aoosp_prt_com_rgbi(uint8_t com);        
// Converts an SAID communication settings to a string (example "LVDS-LOOP-LVDS").
char * aoosp_prt_com_said(uint8_t com);        

// Converts an OSP setup byte to a string (example "pccT-clOU").
char * aoosp_prt_setup(uint8_t flags);         
// Converts a byte array (like a telegram) to a string (example "A0 09 02 00 50 6D").
char * aoosp_prt_bytes(const void * buf, int size ); 
// Converts a channel current setting to a string (example "rshd").
char * aoosp_prt_curchn(uint8_t flags);        
// Converts a SAID I2C configuration to a string (example "itnb").
char * aoosp_prt_i2ccfg(uint8_t flags);        
// Converts a SAID I2C bus speed to bits/second.
int    aoosp_prt_i2ccfg_speed(uint8_t speed);   


#endif


