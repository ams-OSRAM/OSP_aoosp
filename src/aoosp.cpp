// aoosp.cpp - constructs OSP telegrams to send; destructs received OSP telegrams
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
#include <aoosp.h>      // own


// ===== SAID test password =================================================


//#include "../../said-password.h" // pull-in the password if available
#ifndef AOOSP_SAID_TESTPW
  // SAID test password unknown: use the one that generates warnings
  #define AOOSP_SAID_TESTPW   AOOSP_SAID_TESTPW_UNKNOWN
#endif


// Central store for the SAID test password
static uint64_t aoosp_said_testpw = AOOSP_SAID_TESTPW;


/*!
    @brief  Functions that need the SAID test password should obtain it 
            through this central store. 
    @return The SAID password to elevated privileges.
    @note   There are three options to get the correct password in the store
            (1) Compile-time: make sure the macro AOOSP_SAID_TESTPW is defined
                when compiling either by (1a) modify this source file to 
                include the #define, (1b) have said-password.h and uncomment 
                the #include, (1c) add -DAOOSP_SAID_TESTPW in build bypassing
                Arduino IDE.
            (2) Run-time: call `aoosp_said_testpw_set()` early, e.g. in setup().
            (3) Command-time: while running, assuming the command interpreter
                and the command 'said' are linked-in, set the password with 
                (3a) the command 'said password'; this could even (3b) be part 
                of boot.cmd.
            Solutions (1b) and (3a) are the preferred permanent respectively
            temporary solutions. In all cases, ask your contact at ams-OSRAM 
            for the actual password.
    @note   When the SAID receives the correct password it is said to be
            in authenticated state: the host has more privileges. These 
            include changing OTP P2RAM (setotp telegram), executing OTP burn 
            (burn telegram), entering test mode (settestdata telegram).
    @note   When the SAID is in authenticated state, not all feature are 
            operational, like telegram forwarding. To leave authenticated 
            state, set a wrong password, e.g. aoosp_said_testpw_set(0).
    @note   By default, when the test password is not set, it equals
            AOOSP_SAID_TESTPW_UNKNOWN. A warning will be printed by the
            function that sends a password to a SAID (`aoosp_send_settestpw()`)
            if the password being sent equals AOOSP_SAID_TESTPW_UNKNOWN.
            This warns the user if a feature is used that needs the password.
*/
uint64_t aoosp_said_testpw_get() {
  return aoosp_said_testpw;
}


/*!
    @brief  Functions that need the SAID test password (should) obtain it by 
            calling `aoosp_testpw_get()`. However, the store must contain
            the password. Either fill it compile-time, or runtime through a 
            call to this function.
    @note   See `aoosp_said_testpw_get()` for details.
*/
void aoosp_said_testpw_set( uint64_t pw ) {
  aoosp_said_testpw = pw;
}


// ===== init ===============================================================


/*!
    @brief  Initializes the aoosp library.
    @note   At this moment, the aoosp lib has no state (to initialize).
            Present for future extensions that do need initialization.
*/
void aoosp_init() {
  // No init is needed, but already present for future extensions.
  Serial.printf("osp: init\n");
}
