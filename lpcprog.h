/******************************************************************************

Project:           Portable command line ISP for NXP LPC1000 / LPC2000 family
                   and Analog Devices ADUC70xx

Filename:          lpcprog.h

Compiler:          Microsoft VC 6/7, Microsoft VS2008, Microsoft VS2010,
                   GCC Cygwin, GCC Linux, GCC ARM ELF

Author:            Martin Maurer (Martin.Maurer@clibb.de)

Copyright:         (c) Martin Maurer 2003-2014, All rights reserved
Portions Copyright (c) by Aeolus Development 2004 http://www.aeolusdevelopment.com

    This file is part of lpc21isp.

    lpc21isp is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    lpc21isp is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    and GNU General Public License along with lpc21isp.
    If not, see <http://www.gnu.org/licenses/>.
*/

/* LPC_RAMSTART, LPC_RAMBASE
*
* Used in NxpDownload() to decide whether to Flash code or just place in in RAM
* (works for .hex files only)
*
* LPC_RAMSTART - the Physical start address of the SRAM
* LPC_RAMBASE  - the base address where downloading starts.
*                Note that any code in the .hex file that resides in 0x4000,0000 ~ 0x4000,0200
*                will _not_ be written to the LPCs SRAM.
*                This is due to the fact that 0x4000,0040 - 0x4000,0200 is used by the bootrom.
*                Any interrupt vectors must be copied to 0x4000,0000 and remapped to 0x0000,0000
*                by the startup code.
*/
#define LPC_RAMSTART_LPC43XX    0x10000000L
#define LPC_RAMBASE_LPC43XX     0x10000200L

#define LPC_RAMSTART_LPC2XXX    0x40000000L
#define LPC_RAMBASE_LPC2XXX     0x40000200L

#define LPC_RAMSTART_LPC18XX    0x10000000L
#define LPC_RAMBASE_LPC18XX     0x10000200L

#define LPC_RAMSTART_LPC17XX    0x10000000L
#define LPC_RAMBASE_LPC17XX     0x10000200L

#define LPC_RAMSTART_LPC13XX    0x10000000L
#define LPC_RAMBASE_LPC13XX     0x10000300L

#define LPC_RAMSTART_LPC11XX    0x10000000L
#define LPC_RAMBASE_LPC11XX     0x10000300L

#define LPC_RAMSTART_LPC8XX     0x10000000L
#define LPC_RAMBASE_LPC8XX      0x10000270L

/* Return values used by NxpDownload(): reserving all values from 0x1000 to 0x1FFF */

#define NO_ANSWER_WDT       0x1000
#define NO_ANSWER_QM        0x1001
#define NO_ANSWER_SYNC      0x1002
#define NO_ANSWER_OSC       0x1003
#define NO_ANSWER_RBV       0x1004
#define NO_ANSWER_RPID      0x1005
#define ERROR_WRITE_DATA    0x1006
#define ERROR_WRITE_CRC     0x1007
#define ERROR_WRITE_CRC2    0x1008
#define PROGRAM_TOO_LARGE   0x1009

#define USER_ABORT_SYNC     0x100A   /* User aborted synchronisation process */

#define UNKNOWN_LPC         0x100B   /* Unknown LPC detected */

#define UNLOCK_ERROR        0x1100   /* return value is 0x1100 + NXP ISP returned value (0 to 255) */
#define WRONG_ANSWER_PREP   0x1200   /* return value is 0x1200 + NXP ISP returned value (0 to 255) */
#define WRONG_ANSWER_ERAS   0x1300   /* return value is 0x1300 + NXP ISP returned value (0 to 255) */
#define WRONG_ANSWER_WRIT   0x1400   /* return value is 0x1400 + NXP ISP returned value (0 to 255) */
#define WRONG_ANSWER_PREP2  0x1500   /* return value is 0x1500 + NXP ISP returned value (0 to 255) */
#define WRONG_ANSWER_COPY   0x1600   /* return value is 0x1600 + NXP ISP returned value (0 to 255) */
#define FAILED_RUN          0x1700   /* return value is 0x1700 + NXP ISP returned value (0 to 255) */
#define WRONG_ANSWER_BTBNK  0x1800   /* return value is 0x1800 + NXP ISP returned value (0 to 255) */

#if defined COMPILE_FOR_LPC21
#ifndef WIN32
#define LPC_BSL_PIN         13
#define LPC_RESET_PIN       47
#define LPC_RESET(in)       NAsetGPIOpin(LPC_RESET_PIN, (in))
#define LPC_BSL(in)         NAsetGPIOpin(LPC_BSL_PIN,   (in))
#endif // WIN32
#endif // COMPILE_FOR_LPC21


/* LPC_FLASHMASK
*
* LPC_FLASHMASK - bitmask to define the maximum size of the Filesize to download.
*                 LoadFile() will check any new segment address record (03) or extended linear
*                 address record (04) to see if the addressed 64 kByte data block still falls
*                 in the max. flash size.
*                 LoadFile() will not load any files that are larger than this size.
*/
#define LPC_FLASHMASK  0xFFC00000 /* 22 bits = 4 MB */

typedef enum
  {
  CHIP_VARIANT_NONE,
  CHIP_VARIANT_LPC43XX,
  CHIP_VARIANT_LPC2XXX,
  CHIP_VARIANT_LPC18XX,
  CHIP_VARIANT_LPC17XX,
  CHIP_VARIANT_LPC13XX,
  CHIP_VARIANT_LPC11XX,
  CHIP_VARIANT_LPC8XX
  } CHIP_VARIANT;

typedef struct
{
    const unsigned long  id;
    const unsigned long  id2;
    const unsigned int   EvalId2;
    const char *Product;
    const unsigned int   FlashSize;     /* in kiB, for informational purposes only */
    const unsigned int   RAMSize;       /* in kiB, for informational purposes only */
          unsigned int   FlashSectors;  /* total number of sectors */
          unsigned int   MaxCopySize;   /* maximum size that can be copied to Flash in a single command */
    const unsigned int  *SectorTable;   /* pointer to a sector table with constant the sector sizes */
    const CHIP_VARIANT   ChipVariant;
} LPC_DEVICE_TYPE;

int NxpDownload(ISP_ENVIRONMENT *IspEnvironment);

unsigned long ReturnValueLpcRamStart(ISP_ENVIRONMENT *IspEnvironment);

unsigned long ReturnValueLpcRamBase(ISP_ENVIRONMENT *IspEnvironment);

