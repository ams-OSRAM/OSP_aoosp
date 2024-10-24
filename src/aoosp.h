// aoosp.h - constructs OSP telegrams to send; destructs received OSP telegrams
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
#ifndef _AOOSP_H_
#define _AOOSP_H_


// Identifies lib version
#define AOOSP_VERSION "0.4.5"


// Include the (headers of the) modules of this app
#include <aoosp_crc.h>  // computes CRC for OSP telegrams
#include <aoosp_prt.h>  // helpers to pretty print OSP telegrams in human readable form
#include <aoosp_send.h> // send command telegrams (and receive response telegrams)
#include <aoosp_exec.h> // execute high level OSP routines (several telegrams)


// Get the SAID test password - returns AOOSP_SAID_TESTPW_UNKNOWN unless set with eg `aoosp_said_testpw_set()`.
uint64_t aoosp_said_testpw_get();
// Set the SAID test password - functions like aoosp_exec_setotp() need it.
void aoosp_said_testpw_set( uint64_t pw );


// Initializes the aoosp library.
void aoosp_init(); 


#endif



