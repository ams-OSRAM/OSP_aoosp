# OSP Telegrams aoosp

Library "OSP Telegrams aoosp", usually abbreviated to "aoosp",
is one of the **aolibs**; short for Arduino OSP libraries from ams-OSRAM.
This suite implements support for chips that use the Open System Protocol, 
like the AS1163 ("SAID") or the OSIRE E3731i ("RGBi").
The landing page for the _aolibs_ is on 
[GitHub](https://github.com/ams-OSRAM/OSP_aotop).


## Introduction

Library _aoosp_ is at the heart of the dependency graph.

It contains functions to construct telegrams (byte arrays) to be send to OSP nodes.
It also contains functions to destruct response telegrams received back from OSP nodes.
Both functions are hidden behind the API; the API allows sending and receiving telegrams,
which involves constructing, transmit-out, transmit-in and destructing.

For telegram transmission this library relies on a communications layer, typically _aospi_.
That library expects a board like the **OSP32** board in the evaluation kit.

![aoosp in context](extras/aolibs-aoosp.drawio.png)


## Examples

This library comes with the following examples.
You can find them in the Arduino IDE via 
File > Examples > OSP Telegrams aoosp > ...

- **aoosp_crc** ([source](examples/aoosp_crc))  
  This demo computes the CRC of a telegram (byte buffer).
  Once to compute the required CRC for sending a command.
  Once to check the CRC in an incoming response.

- **aoosp_min** ([source](examples/aoosp_min))  
  This demo sends a minimal set of OSP telegrams that light up an LED
  connected to a SAID channel. Once using BiDir, once using Loop.
  Then repeats.

- **aoosp_topo** ([source](examples/aoosp_topo))  
  This demo scans all nodes and prints out the chain configuration:
  comms for both SIOs, direction, power state, number of RGBs and I2Cs.

- **aoosp_cur** ([source](examples/aoosp_cur))  
  This demo configures channel 1 of node 001 to use a high drive current,
  and it configures channel 1 of node 002 to use a low drive current.
  Next it broadcasts `setpwm` with max brightness (7FFF 7FFF 7FFF). This
  results in the two white lights with different brightness.

- **aoosp_error** ([source](examples/aoosp_error))  
  This demo shows how error handling works in SAID. The STAT register has 
  error flags denoting which errors occurred. The SETUP register has flags
  denoting which errors need to be trapped. When an error occurs 
  (is flagged in STAT) whose trap bit is set (in SETUP), then the error
  is handled by switching to SLEEP state. This switches off the PWMs. 

- **aoosp_group** ([source](examples/aoosp_group))  
  This demo groups two SAIDs, then sends PWM commands to that group.
  One command to switch the group to red, one command to switch the 
  group to green. Then repeats.

- **aoosp_i2c** ([source](examples/aoosp_i2c))  
  This demo first performs an I2C scan using the I2C bridge in a SAID.
  Then it issues I2C read and write transactions to an EEPROM memory,
  assumed to have I2C device address 0x50 connected to the first SAID.
  Finally it polls the INT line and shows its status on SAID1.RGB0.

- **aoosp_sync** ([source](examples/aoosp_sync))  
  This demo shows how to use the SYNC feature; a feature that switches on all 
  LEDs at the same time. We first enable (channels) of nodes for SYNC
  (AOOSP_CURCHN_FLAGS_SYNCEN). Next, set those channels to some PWM value.
  Finally we issue a SYNC by broadcasting the SYNC telegram. This makes
  all (channels) of all nodes that are configured for SYNC to activate their
  PWM settings. 
  
  It is also possible to pulse a pin as hardware SYNC. The demo has that
  as secondary feature, but The _OTP password must be known and enabled_ 
  for this to work.

- **aoosp_ledst** ([source](examples/aoosp_ledst))  
  This demo shows the behavior of LOS (LED Open Short) status which is 
  obtained via the telegram `readledst[chn]`. 

- **aoosp_time** ([source](examples/aoosp_time))  
  This demo sends a series of telegrams. The series originally comes from 
  `aomw_topo_build`. We measure how long it takes to run that series.
  Once we use the high level aoosp API, and once we use low level aospi API.
  The conclusion is that the software overhead can be ignored.

- **aoosp_cluster** ([source](examples/aoosp_cluster))  
  This demo shows how clustering works - even on the evaluation kit,
  where none of the SAIDs is wired for clustering.
  The _OTP password must be known and enabled_ or this example will not work.

- **aoosp_tinfo** ([source](examples/aoosp_tinfo))  
  This demo shows how to use the ASKTINFO feature. The `asktinfo` telegram 
  collects the minimum and maximum temperature over an OSP chain using a 
  mechanism called "serial cast". This demo also explains serial cast.

- **aoosp_otp** ([source](examples/aoosp_otp))  
  This demo reads and writes from/to the OTP (one time programmable 
  memory) of a SAID.
  The _OTP password must be known and enabled_ or this example will not work.
  

## Module architecture

This library contains 4 modules, see figure below (arrows indicate `#include`).

![Modules](extras/aoosp-modules.drawio.png)

- **aoosp_crc** (`aoosp_crc.cpp` and `aoosp_crc.h`) is a small module that implements 
  one function. It computes the OSP CRC checksum of a byte array. The module is stateless.
  
- **aoosp_prt** (`aoosp_prt.cpp` and `aoosp_prt.h`) is a slightly bigger module that implements 
  several functions to pretty print telegrams. It is normally not used in production code.
  It is extensively used by the logging feature of module `aoosp_send`.

- **aoosp_send** (`aoosp_send.cpp` and `aoosp_send.h`) is the _core_ module of the library.
  It contains functions that send commands and receive the responses if there is any.
  The module supports both RGBi's and SAIDs. Not all telegrams are implemented yet.
  The module is stateless, with one exception: it records if logging is enabled or disabled.

- **aoosp_exec** (`aoosp_exec.cpp` and `aoosp_exec.h`) is contains higher level features, 
  which typically send multiple telegrams. A very prominent example is `aoosp_exec_resetinit()`
  which sends a RESET and INIT telegram, but auto detects if BiDir (terminator) or 
  Loop (cable) is configured. Other high level functions help in accesses I2C devices 
  connected to the SAID, or the OTP memory inside the SAID. Also stateless.
   
Each module has its own header file, but the library has an overarching header `aoosp.h`,
which includes the module headers. It is suggested that users just include the overarching 
header.


## API

The header [aoosp.h](src/aoosp.h) contains the API of this library.
It includes the module headers [aoosp_crc.h](src/aoosp_crc.h), [aoosp_prt.h](src/aoosp_prt.h), 
[aoosp_send.h](src/aoosp_send.h) and [aoosp_exec.h](src/aoosp_exec.h).
The headers contain little documentation; for that see the module source files. 


### aoosp

- `aoosp_init()` not really needed (aoosp has no state to init), but added for forward compatibility.
- `AOOSP_VERSION`  identifies the version of the library.

The module also implements the rarely used store for the SAID password to enter test mode,
see `aoosp_said_testpw_get()` and `aoosp_said_testpw_set(...)`.


### aoosp_crc

- `aoosp_crc(...)` the only function implemented in this modules; computes the CRC of a byte array.


### aoosp_prt

Contains several functions to print telegram fields more user friendly. Examples are:

- `aoosp_prt_temp_said(...)` to convert a SAID raw temperature to Celsius.
- `aoosp_prt_stat_state(...)` to convert a node stat to a string describing the power state.
- `aoosp_prt_bytes(...)` to convert a byte array (like a telegram) to a string.
- ...

These functions are publicly accessible, but are intended for logging in this library.

Warning: pretty print functions that return a `char *` all use the same global 
character buffer  for returning the string. This means that only _one_ such function 
can be used at a time. For example this will _not_ work:
 
```
  printf("%s-%s", aoosp_prt_stat_rgbi(x), aoosp_prt_stat_said(x) ); // ERROR: double prt
```


### aoosp_send

This is the core of the library. In principle it contains one function per telegram type.
Examples are

- `aoosp_send_reset(addr)`         resets all nodes in the chain (all "off"; they also lose their address).
- `aoosp_send_initloop(addr,...)`  assigns an address to each node; also configures all nodes for Loop.
- `aoosp_send_clrerror(addr)`      clears the error flags of the addressed node.
- `aoosp_send_goactive(addr)`      switches the power state of the addressed node to active ("on").
- `aoosp_send_setpwmchn(addr,...)` configures the PWM settings of one channel of the addressed node ("lit").
- ...

It should be noted that some telegrams come in variants like `setpwm` and `setpwmchn`.
The `...chn` variant is for nodes with multiple RGB channels, like SAID, the
other variant is for nodes with one channel, like RGBI.

There are several macros to help composing telegrams or analyzing responses.
- `AOOSP_ADDR_XXX` to pass (`AOOSP_ADDR_BROADCAST`, `AOOSP_ADDR_GROUP5`) or check (`AOOSP_ADDR_ISUNICAST`) addresses.
- To analyze identity, e.g. `AOOSP_IDENTIFY_IS_SAID`.
- To analyze various status bytes, e.g. `AOOSP_STAT_FLAGS_OV`, `AOOSP_COMST_SIO1_MCU`, `AOOSP_SETUP_FLAGS_UV`.
- To configure current drivers, e.g. `AOOSP_CURCHN_FLAGS_DITHER`, or `AOOSP_CURCHN_FLAGS_SYNCEN`.
- To configure/test I2C, e.g. `AOOSP_I2CCFG_SPEED_400kHz`, or `AOOSP_I2CCFG_FLAGS_NACK`.

This module supports logging. 
Via `aoosp_loglevel_set(level)` and `aoosp_loglevel_get()` the log level can be managed:

- `aoosp_loglevel_none` nothing is logged (default).
- `aoosp_loglevel_args` logging of sent and received telegram arguments.
- `aoosp_loglevel_tele` also logs raw (sent and received) telegram bytes.


### aoosp_exec

Some operations on OSP nodes require multiple telegrams.
Some frequent ones have been abstracted in this module.

- `aoosp_exec_resetinit()`            sends RESET and INIT telegrams, auto detecting BiDir or Loop.
- `aoosp_exec_resetinit_last()`       returns the address of the last node as determined by the last call to `aoosp_exec_resetinit()`.
- `aoosp_exec_otpdump(...)`           reads the entire OTP (mirror) and prints details requested in flags.
- `aoosp_exec_setotp(...)`            updates one byte in the OTP (mirror).
- `aoosp_exec_i2cenable_get(...)`     reads the I2C_BRIDGE_EN bit in OTP (mirror).
- `aoosp_exec_i2cenable_set(...)`     writes the I2C_BRIDGE_EN bit in OTP (mirror).
- `aoosp_exec_i2cpower(...)`          checks if the SAID has an I2C bridge, if so, powers the I2C bus.
- `aoosp_exec_syncpinenable_get(...)` reads the SYNC_PIN_EN bit from OTP (mirror).
- `aoosp_exec_syncpinenable_set(...)` writes the SYNC_PIN_EN bit to OTP (mirror).
- `aoosp_exec_i2cwrite8(...)`         writes to an I2C device connected to a SAID with I2C bridge.
- `aoosp_exec_i2cread8(...)`          reads from an I2C device connected to a SAID with I2C bridge.


## Version history _aoosp_

- **2024 November 29, 0.5.0**
  - Added example `aoosp_ledst.ino`.
  - `aoosp_exec_setotp()` now uses new `aoosp_send_settestpw_sr()`.
  - Added telegram `aoosp_send_settestpw_sr()`.
  - Explicit errors in `aoosp_otp.ino`.
  - Added telegrams `aoosp_send_readledst()` and `aoosp_send_readledstchn()`.
  - Added `aoosp_prt_ledst()`; all aoosp_prt now use "-" as field separator.
  - Added telegram `aoosp_send_godeepsleep()`.
  - Added example `aoosp_tinfo` explaining serial cast (with ASKTINFO).
  - Added telegrams `aoosp_con_asktinfo()` and variant `aoosp_con_asktinfo_init()`.
  - Added API documentation for `aoosp_loglevel_set()` and `aoosp_loglevel_get()`.
  - Removed old datasheet names for telegrams (in `aoosp_send.cpp`).

- **2024 October 24, 0.4.5**
  - Added example `aoosp_cluster.ino`.
  - Documented (telegram) macros in readme.
  - SAID test password now in a store `aoosp_said_testpw_get()` and `aoosp_said_testpw_set()`.
  - OTP write `aoosp_exec_setotp()` uses that store.

- **2024 October 22, 0.4.4**
  - Response telegrams now checked for mismatch between payload size and PSI.
  
- **2024 October 8, 0.4.3**
  - Prefixed `modules.drawio.png` with library short name.
  - Documentation update in: `readme.md`, `aoosp_exec.cpp`, `aoosp_prt.cpp`, and `aoosp_send.cpp`.
  - Added example `aoosp_cur.ino` (drive current demo).
  - Fixed white lines in `aoosp_exec.h`.
  - Log of `aoosp_send_initxxx()` now prints 3 digits.
  - Moved domain from `github.com/ams-OSRAM-Group` to `github.com/ams-OSRAM`.

- **2024 September 10, 0.4.2**
  - Fixed TID bug in `aoosp_con_seti2ccfg()`.
  - `aoosp_exec_i2cpower(addr)` now also checks if addr is a SAID.
  - Detailed addressing for `aoosp_send_xxx()` and removed old datasheet names.
  - Corrected documentation on `addr` in `aoosp_exec_xxx()` functions.
  - Added topology check to `aoosp-time.ino`.
  - Added explanation for SYNC via pin in `aoosp_sync.ino`.
  - Added remark on OTP password for `aoosp_otp.ino`.
  - I2C in `aoop_i2c.ino` description updated and device address changed to match OSP32.
  - Now also powering I2C bus in `aoosp_i2c.ino` for EEPROM.
  - Role of 'mult' in `aoosp_error.ino` explained.
  - Added BEHAVIOR section to explanation in examples.

- **2024 September 5, 0.4.1**
  - API section in readme now shows parameter names.
  - Added `aoosp_exec_resetinit_last()`.
  - Made output parameters of `aoosp_exec_resetinit()` optional (adapted an example).
  - Bug fix: when `aoosp_exec_resetinit` had NULL for output parameters they were still assigned.
  - Bug fix: changed `AOOSP_ADDR_NOTNIT` to `AOOSP_ADDR_UNINIT` in `AOOSP_ADDR_GROUP`.
  
- **2024 August 28, 0.4.0**
  - Renamed/split `AOOSP_ADDR_MIN|MAX` to `AOOSP_ADDR_UNICASTMIN|MAX` and `AOOSP_ADDR_GLOBALMIN|MAX`.
  - Added `AOOSP_OTPADDR_CUSTOMER_MIN|MAX`.
  - Added links in `readme.md` for all example sketches.

- **2024 August 9, 0.3.1**
  - Typos fixed in `aoosp_send.cpp`.
  - In `aoosp_send.h` cleaned up `AOOSP_ADDR_xxx`.
  - Typos fixed in `readme.md`, `aoosp.cpp`, `aoosp_crc.cpp`, `aoosp_exec.h`.
  - Typos fixed in `aoosp_i2c.ino`, `aoosp_sync.ino`, `aoosp_topo.ino`.
  - Typos fixed in `aoosp_exec.cpp`.
  
- **2024 August 5, 0.3.0**  
  - In `aoosp_send.h` all non-implemented TIDs tagged as not-yet-implemented or reserved.
  - Corrected link to GitHub from aotop to `OSP_aotop`.
  - Remove "oalib" from `sentence=` in `library.properties`.
  - Added `#include "../../said-password.h"`.
  - Added `aoosp_exec_syncpinenable_get()` and `aoosp_exec_syncpinenable_set()` and hw sync in `aoosp_sync.ino`.
  - Arduino name changed from `OSP Telegrams - aoosp` to `OSP Telegrams aoosp`.
  - Renamed dir `extra` to `extras`.
  - `license.txt`, `examples\xxx.ino` line endings changed from LF to CR+LF.
  - Fixed argument test in `aoosp_con_setcurchn()`.

- **2024 July 02, 0.2.4**  
  - Initial release candidate.


(end)

