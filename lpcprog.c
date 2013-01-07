/******************************************************************************

Project:           Portable command line ISP for NXP LPC1000 / LPC2000 family
                   and Analog Devices ADUC70xx

Filename:          lpcprog.c

Compiler:          Microsoft VC 6/7, Microsoft VS2008, Microsoft VS2010,
                   GCC Cygwin, GCC Linux, GCC ARM ELF

Author:            Martin Maurer (Martin.Maurer@clibb.de)

Copyright:         (c) Martin Maurer 2003-2011, All rights reserved
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

// This file is for the Actual Programming of the LPC Chips

#if defined(_WIN32)
#if !defined __BORLANDC__
#include "StdAfx.h"
#endif
#endif // defined(_WIN32)
#include "lpc21isp.h"

#ifdef LPC_SUPPORT
#include "lpcprog.h"

static const unsigned int SectorTable_210x[] =
{
    8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
    8192, 8192, 8192, 8192, 8192, 8192, 8192
};

static const unsigned int SectorTable_2103[] =
{
    4096, 4096, 4096, 4096, 4096, 4096, 4096, 4096
};

static const unsigned int SectorTable_2109[] =
{
    8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192
};

static const unsigned int SectorTable_211x[] =
{
    8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
    8192, 8192, 8192, 8192, 8192, 8192, 8192,
};

static const unsigned int SectorTable_212x[] =
{
    8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
    65536, 65536, 8192, 8192, 8192, 8192, 8192, 8192, 8192
};

// Used for devices with 500K (LPC2138 and LPC2148) and
// for devices with 504K (1 extra 4k block at the end)
static const unsigned int SectorTable_213x[] =
{
     4096,  4096,  4096,  4096,  4096,  4096,  4096,  4096,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768,  4096,  4096,
     4096,  4096,  4096,  4096
};

// Used for LPC17xx devices
static const unsigned int SectorTable_17xx[] =
{
     4096,  4096,  4096,  4096,  4096,  4096,  4096,  4096,
     4096,  4096,  4096,  4096,  4096,  4096,  4096,  4096,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768
};

static int unsigned SectorTable_RAM[]  = { 65000 };

static LPC_DEVICE_TYPE LPCtypes[] =
{
   { 0, 0, 0, 0, 0, 0, 0, CHIP_VARIANT_NONE },  /* unknown */

   // id, name of product, flash size, ram size, total number of sector, max copy size, sector table, chip variant

   { 0x2500102B, "1102",         32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },

   { 0x0A07102B, "1110.../002",   4,  1,  1, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x1A07102B, "1110.../002",   4,  1,  1, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0A16D02B, "1111.../002",   8,  2,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x1A16D02B, "1111.../002",   8,  2,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x041E502B, "1111.../101",   8,  2,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2516D02B, "1111.../102",   8,  2,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0416502B, "1111.../201",   8,  4,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2516902B, "1111.../202",   8,  4,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x042D502B, "1112.../101",  16,  2,  4, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2524D02B, "1112.../102",  16,  2,  4, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0A24902B, "1112.../102",  16,  4,  4, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x1A24902B, "1112.../102",  16,  4,  4, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0425502B, "1112.../201",  16,  4,  4, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2524902B, "1112.../202",  16,  4,  4, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0434502B, "1113.../201",  24,  4,  6, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2532902B, "1113.../202",  24,  4,  6, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0434102B, "1113.../301",  24,  8,  6, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2532102B, "1113.../302",  24,  8,  6, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0A40902B, "1114.../102",  32,  4,  8, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x1A40902B, "1114.../102",  32,  4,  8, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0444502B, "1114.../201",  32,  4,  8, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2540902B, "1114.../202",  32,  4,  8, 1024, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0444102B, "1114.../301",  32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x2540102B, "1114.../302",  32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },

   { 0x1421102B, "11C12.../301",  16,  8,  4, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x1440102B, "11C14.../301",  32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x1431102B, "11C22.../301",  16,  8,  4, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x1430102B, "11C24.../301",  32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },

   { 0x0364002B, "1224.../101",  32,  8,  4, 2048, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0364202B, "1224.../121",  48, 12, 32, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0365002B, "1225.../301",  64, 16, 32, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0365202B, "1225.../321",  80, 20, 32, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0366002B, "1226",         96, 24, 32, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },
   { 0x0367002B, "1227",        128, 32, 32, 4096, SectorTable_17xx, CHIP_VARIANT_LPC11XX },

   { 0x2C42502B, "1311",          8,  4,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC13XX },
   { 0x1816902B, "1311/01",       8,  4,  2, 1024, SectorTable_17xx, CHIP_VARIANT_LPC13XX },
   { 0x2C40102B, "1313",         32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC13XX },
   { 0x1830102B, "1313/01",      32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC13XX },
   { 0x3D01402B, "1342",         16,  4,  4, 1024, SectorTable_17xx, CHIP_VARIANT_LPC13XX },
   { 0x3D00002B, "1343",         32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC13XX },

   { 0x25001118, "1751",         32,  8,  8, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x25001121, "1752",         64, 16, 16, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x25011722, "1754",        128, 32, 18, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x25011723, "1756",        256, 32, 22, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x25013F37, "1758",        512, 64, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x25113737, "1759",        512, 64, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x26011922, "1764",        128, 32, 18, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x26013733, "1765",        256, 64, 22, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x26013F33, "1766",        256, 64, 22, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x26012837, "1767",        512, 64, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x26013F37, "1768",        512, 64, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x26113F37, "1769",        512, 64, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },

   { 0x27011132, "1774",        128, 40, 18, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x27191F43, "1776",        256, 80, 22, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x27193747, "1777",        512, 96, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x27193F47, "1778",        512, 96, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x281D1743, "1785",        256, 80, 22, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x281D1F43, "1786",        256, 80, 22, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x281D3747, "1787",        512, 96, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },
   { 0x281D3F47, "1788",        512, 96, 30, 4096, SectorTable_17xx, CHIP_VARIANT_LPC17XX },

   { 0x0004FF11, "2103",         32,  8,  8, 4096, SectorTable_2103, CHIP_VARIANT_LPC2XXX },
   { 0xFFF0FF12, "2104",        128, 16, 15, 8192, SectorTable_210x, CHIP_VARIANT_LPC2XXX },
   { 0xFFF0FF22, "2105",        128, 32, 15, 8192, SectorTable_210x, CHIP_VARIANT_LPC2XXX },
   { 0xFFF0FF32, "2106",        128, 64, 15, 8192, SectorTable_210x, CHIP_VARIANT_LPC2XXX },
   { 0x0201FF01, "2109",         64,  8,  8, 4096, SectorTable_2109, CHIP_VARIANT_LPC2XXX },
   { 0x0101FF12, "2114",        128, 16, 15, 8192, SectorTable_211x, CHIP_VARIANT_LPC2XXX },
   { 0x0201FF12, "2119",        128, 16, 15, 8192, SectorTable_211x, CHIP_VARIANT_LPC2XXX },
   { 0x0101FF13, "2124",        256, 16, 17, 8192, SectorTable_212x, CHIP_VARIANT_LPC2XXX },
   { 0x0201FF13, "2129",        256, 16, 17, 8192, SectorTable_212x, CHIP_VARIANT_LPC2XXX },
   { 0x0002FF01, "2131",         32,  8,  8, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0002FF11, "2132",         64, 16,  9, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0002FF12, "2134",        128, 16, 11, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0002FF23, "2136",        256, 32, 15, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0002FF25, "2138",        512, 32, 27, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0402FF01, "2141",         32,  8,  8, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0402FF11, "2142",         64, 16,  9, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0402FF12, "2144",        128, 16, 11, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0402FF23, "2146",        256, 40, 15, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0402FF25, "2148",        512, 40, 27, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0301FF13, "2194",        256, 16, 17, 8192, SectorTable_212x, CHIP_VARIANT_LPC2XXX },
   { 0x0301FF12, "2210",          0, 16,  0, 8192, SectorTable_211x, CHIP_VARIANT_LPC2XXX }, /* table is a "don't care" */
   { 0x0401FF12, "2212",        128, 16, 15, 8192, SectorTable_211x, CHIP_VARIANT_LPC2XXX },
   { 0x0601FF13, "2214",        256, 16, 17, 8192, SectorTable_212x, CHIP_VARIANT_LPC2XXX },
   /*            "2290"; same id as the LPC2210 */
   { 0x0401FF13, "2292",        256, 16, 17, 8192, SectorTable_212x, CHIP_VARIANT_LPC2XXX },
   { 0x0501FF13, "2294",        256, 16, 17, 8192, SectorTable_212x, CHIP_VARIANT_LPC2XXX },
   { 0x00000000, "2361",        128, 34, 11, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x00000000, "2362",        128, 34, 11, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0603FB02, "2364",        128, 34, 11, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX }, /* From UM10211 Rev. 01 -- 6 July 2007 */
   { 0x1600F902, "2364",        128, 34, 11, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1600E823, "2365",        256, 58, 15, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0603FB23, "2366",        256, 58, 15, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX }, /* From UM10211 Rev. 01 -- 6 July 2007 */
   { 0x1600F923, "2366",        256, 58, 15, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1600E825, "2367",        512, 58, 15, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0603FB25, "2368",        512, 58, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX }, /* From UM10211 Rev. 01 -- 6 July 2007 */
   { 0x1600F925, "2368",        512, 58, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1700E825, "2377",        512, 58, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x0703FF25, "2378",        512, 58, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX }, /* From UM10211 Rev. 01 -- 6 July 2007 */
   { 0x1600FD25, "2378",        512, 58, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX }, /* From UM10211 Rev. 01 -- 29 October 2007 */
   { 0x1700FD25, "2378",        512, 58, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1700FF35, "2387",        512, 98, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX }, /* From UM10211 Rev. 03 -- 25 August 2008 */
   { 0x1800F935, "2387",        512, 98, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1800FF35, "2388",        512, 98, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1500FF35, "2458",        512, 98, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1600FF30, "2460",          0, 98,  0, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1600FF35, "2468",        512, 98, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1701FF30, "2470",          0, 98,  0, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX },
   { 0x1701FF35, "2478",        512, 98, 28, 4096, SectorTable_213x, CHIP_VARIANT_LPC2XXX }
};

/***************************** NXP Download *********************************/
/**  Download the file from the internal memory image to the NXP microcontroller.
*   This function is visible from outside if COMPILE_FOR_LPC21
*/

static int SendAndVerify(ISP_ENVIRONMENT *IspEnvironment, const char *Command,
                                 char *AnswerBuffer, int AnswerLength)
{
    unsigned long realsize;
    int cmdlen;

    SendComPort(IspEnvironment, Command);
    ReceiveComPort(IspEnvironment, AnswerBuffer, AnswerLength - 1, &realsize, 2, 5000);
    cmdlen = strlen(Command);
    return (strncmp(AnswerBuffer, Command, cmdlen) == 0
        && strcmp(AnswerBuffer + cmdlen, "0\r\n") == 0);
}



/***************************** NxpOutputErrorMessage ***********************/
/**  Given an error number find and print the appropriate error message.
\param [in] ErrorNumber The number of the error.
*/
#if defined COMPILE_FOR_LPC21

#define NxpOutputErrorMessage(in)        // Cleanly remove this feature from the embedded version !!

#else

static void NxpOutputErrorMessage(unsigned char ErrorNumber)
{
    switch (ErrorNumber)
    {
    case   0:
        DebugPrintf(1, "CMD_SUCCESS\n");
        break;

    case   1:
        DebugPrintf(1, "INVALID_COMMAND\n");
        break;

    case   2:
        DebugPrintf(1, "SRC_ADDR_ERROR: Source address is not on word boundary.\n");
        break;
    case   3:
        DebugPrintf(1, "DST_ADDR_ERROR: Destination address is not on a correct boundary.\n");
        break;

    case   4:
        DebugPrintf(1, "SRC_ADDR_NOT_MAPPED: Source address is not mapped in the memory map.\n"
                       "                     Count value is taken into consideration where applicable.\n");
        break;

    case   5:
        DebugPrintf(1, "DST_ADDR_NOT_MAPPED: Destination address is not mapped in the memory map.\n"
                       "                     Count value is taken into consideration where applicable.\n");
        break;

    case   6:
        DebugPrintf(1, "COUNT_ERROR: Byte count is not multiple of 4 or is not a permitted value.\n");
        break;

    case   7:
        DebugPrintf(1, "INVALID_SECTOR: Sector number is invalid or end sector number is\n"
                       "                greater than start sector number.\n");
        break;

    case   8:
        DebugPrintf(1, "SECTOR_NOT_BLANK\n");
        break;

    case   9:
        DebugPrintf(1, "SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION:\n"
                       "Command to prepare sector for write operation was not executed.\n");
        break;

    case  10:
        DebugPrintf(1, "COMPARE_ERROR: Source and destination data not equal.\n");
        break;

    case  11:
        DebugPrintf(1, "BUSY: Flash programming hardware interface is busy.\n");
        break;

    case  12:
        DebugPrintf(1, "PARAM_ERROR: Insufficient number of parameters or invalid parameter.\n");
        break;

    case  13:
        DebugPrintf(1, "ADDR_ERROR: Address is not on word boundary.\n");
        break;

    case  14:
        DebugPrintf(1, "ADDR_NOT_MAPPED: Address is not mapped in the memory map.\n"
                       "                 Count value is taken in to consideration where applicable.\n");
        break;

    case  15:
        DebugPrintf(1, "CMD_LOCKED\n");
        break;

    case  16:
        DebugPrintf(1, "INVALID_CODE: Unlock code is invalid.\n");
        break;

    case  17:
        DebugPrintf(1, "INVALID_BAUD_RATE: Invalid baud rate setting.\n");
        break;

    case  18:
        DebugPrintf(1, "INVALID_STOP_BIT: Invalid stop bit setting.\n");
        break;

    case  19:
        DebugPrintf( 1, "CODE READ PROTECTION ENABLED\n");
        break;

    case 255:
        break;

    default:
        DebugPrintf(1, "unknown error %u\n", ErrorNumber);
        break;
    }

    //DebugPrintf(1, "error (%u), see  NxpOutputErrorMessage() in lpc21isp.c for help \n\r", ErrorNumber);
}
#endif // !defined COMPILE_FOR_LPC21

/***************************** GetAndReportErrorNumber ***************************/
/**  Find error number in string.  This will normally be the string
returned from the microcontroller.
\param [in] Answer the buffer to search for the error number.
\return the error number found, if no linefeed found before the end of the
string an error value of 255 is returned. If a non-numeric value is found
then it is printed to stdout and an error value of 255 is returned.
*/
static unsigned char GetAndReportErrorNumber(const char *Answer)
{
    unsigned char Result = 0xFF;                            // Error !!!
    unsigned int i = 0;

    while (1)
    {
        if (Answer[i] == 0x00)
        {
            break;
        }

        if (Answer[i] == 0x0a)
        {
            i++;

            if (Answer[i] < '0' || Answer[i] > '9')
            {
                DebugPrintf(1, "ErrorString: %s", &Answer[i]);
                break;
            }

            Result = (unsigned char) (atoi(&Answer[i]));
            break;
        }

        i++;
    }

    NxpOutputErrorMessage(Result);

    return Result;
}


int NxpDownload(ISP_ENVIRONMENT *IspEnvironment)
{
    unsigned long realsize;
    char Answer[128];
    char ExpectedAnswer[128];
    char temp[128];
    /*const*/ char *strippedAnswer, *endPtr;
    int  strippedsize;
    int nQuestionMarks;
    int found;
    unsigned long Sector;
    unsigned long SectorLength;
    unsigned long SectorStart, SectorOffset, SectorChunk;
    char tmpString[128];
    char uuencode_table[64];
    int Line;
    unsigned long tmpStringPos;
    unsigned long BlockOffset;
    unsigned long Block;
    unsigned long Pos;
    unsigned long CopyLength;
    int c,k=0,i;
    unsigned long ivt_CRC;          // CRC over interrupt vector table
    unsigned long block_CRC;
    time_t tStartUpload=0, tDoneUpload=0;
    char tmp_string[64];
    char * cmdstr;

#if !defined COMPILE_FOR_LPC21

#if defined __BORLANDC__
#define local_static static
#else
#define local_static
#endif

//    char * cmdstr;
    int repeat = 0;
    // Puffer for data to resend after "RESEND\r\n" Target responce
    local_static char sendbuf0[128];
    local_static char sendbuf1[128];
    local_static char sendbuf2[128];
    local_static char sendbuf3[128];
    local_static char sendbuf4[128];
    local_static char sendbuf5[128];
    local_static char sendbuf6[128];
    local_static char sendbuf7[128];
    local_static char sendbuf8[128];
    local_static char sendbuf9[128];
    local_static char sendbuf10[128];
    local_static char sendbuf11[128];
    local_static char sendbuf12[128];
    local_static char sendbuf13[128];
    local_static char sendbuf14[128];
    local_static char sendbuf15[128];
    local_static char sendbuf16[128];
    local_static char sendbuf17[128];
    local_static char sendbuf18[128];
    local_static char sendbuf19[128];

    char * sendbuf[20] = {    sendbuf0,  sendbuf1,  sendbuf2,  sendbuf3,  sendbuf4,
                              sendbuf5,  sendbuf6,  sendbuf7,  sendbuf8,  sendbuf9,
                              sendbuf10, sendbuf11, sendbuf12, sendbuf13, sendbuf14,
                              sendbuf15, sendbuf16, sendbuf17, sendbuf18, sendbuf19};
#endif

    DebugPrintf(2, "Synchronizing (ESC to abort)");

    PrepareKeyboardTtySettings();

#if defined INTEGRATED_IN_WIN_APP
    if (IspEnvironment->NoSync)
    {
        found = 1;
    }
    else
#endif
    {
        for (nQuestionMarks = found = 0; !found && nQuestionMarks < IspEnvironment->nQuestionMarks; nQuestionMarks++)
        {
#if defined INTEGRATED_IN_WIN_APP
            // allow calling application to abort when syncing takes too long

            if (!AppSyncing(nQuestionMarks))
            {
                return (USER_ABORT_SYNC);
            }
#else
#ifndef Exclude_kbhit
            if (kbhit())
            {
                if (getch() == 0x1b)
                {
                    ResetKeyboardTtySettings();
                    DebugPrintf(2, "\nUser aborted during synchronisation\n");
                    return (USER_ABORT_SYNC);
                }
            }
#endif
#endif

            DebugPrintf(2, ".");
            SendComPort(IspEnvironment, "?");

            memset(Answer,0,sizeof(Answer));
            ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 1,100);

            strippedAnswer = Answer;
            strippedsize = realsize;
            while ((strippedsize > 0) && ((*strippedAnswer == '?') || (*strippedAnswer == 0)))
            {
                strippedAnswer++;
                strippedsize--;
            }

            sprintf(tmp_string, "StrippedAnswer(Length=%d): '", strippedsize);
            DumpString(3, strippedAnswer, strippedsize, tmp_string);

            tStartUpload = time(NULL);

            if (strcmp(strippedAnswer, "Synchronized\r\n") == 0)
            {
                found = 1;
            }
#if !defined COMPILE_FOR_LPC21
            else
            {
                ResetTarget(IspEnvironment, PROGRAM_MODE);
            }
#endif
        }
    }

    ResetKeyboardTtySettings();

    if (!found)
    {
        DebugPrintf(1, " no answer on '?'\n");
        return (NO_ANSWER_QM);
    }

#if defined INTEGRATED_IN_WIN_APP
    AppSyncing(-1);                         // flag syncing done
#endif

    DebugPrintf(2, " OK\n");

    SendComPort(IspEnvironment, "Synchronized\n");

    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer) - 1, &realsize, 2, 1000);

    if ((strcmp(Answer, "Synchronized\r\nOK\r\n") != 0) && (strcmp(Answer, "Synchronized\rOK\r\n") != 0) &&
        (strcmp(Answer, "Synchronized\nOK\r\n") != 0))
    {
        DebugPrintf(1, "No answer on 'Synchronized'\n");
        return (NO_ANSWER_SYNC);
    }

    DebugPrintf(3, "Synchronized 1\n");

    DebugPrintf(3, "Setting oscillator\n");

    sprintf(temp, "%s\n", IspEnvironment->StringOscillator);

    SendComPort(IspEnvironment, temp);

    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 2, 1000);

    sprintf(temp, "%s\nOK\r\n", IspEnvironment->StringOscillator);

    if (strcmp(Answer, temp) != 0)
    {
        DebugPrintf(1, "No answer on Oscillator-Command\n");
        return (NO_ANSWER_OSC);
    }

    DebugPrintf(3, "Unlock\n");

    cmdstr = "U 23130\n";

    if (!SendAndVerify(IspEnvironment, cmdstr, Answer, sizeof Answer))
    {
        DebugPrintf(1, "Unlock-Command:\n");
        return (UNLOCK_ERROR + GetAndReportErrorNumber(Answer));
    }

    DebugPrintf(2, "Read bootcode version: ");

    cmdstr = "K\n";

    SendComPort(IspEnvironment, cmdstr);

    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 4,5000);

    if (strncmp(Answer, cmdstr, strlen(cmdstr)) != 0)
    {
        DebugPrintf(1, "no answer on Read Boot Code Version\n");
        return (NO_ANSWER_RBV);
    }

    if (strncmp(Answer + strlen(cmdstr), "0\r\n", 3) == 0)
    {
        strippedAnswer = Answer + strlen(cmdstr) + 3;
        /*
        int maj, min, build;
        if (sscanf(strippedAnswer, "%d %d %d", &build, &min, &maj) == 2) {
        maj = min;
        min = build;
        build = 0;
        } // if
        DebugPrintf(2, "%d.%d.%d\n", maj, min, build);
        */
        DebugPrintf(2, strippedAnswer);
    }
    else
    {
        DebugPrintf(2, "unknown\n");
    }

    DebugPrintf(2, "Read part ID: ");

    cmdstr = "J\n";

    SendComPort(IspEnvironment, cmdstr);

    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 3,5000);

    if (strncmp(Answer, cmdstr, strlen(cmdstr)) != 0)
    {
        DebugPrintf(1, "no answer on Read Part Id\n");
        return (NO_ANSWER_RPID);
    }

    strippedAnswer = (strncmp(Answer, "J\n0\r\n", 5) == 0) ? Answer + 5 : Answer;

    Pos = strtoul(strippedAnswer, &endPtr, 10);
    *endPtr = '\0'; /* delete \r\n */
    for (i = sizeof LPCtypes / sizeof LPCtypes[0] - 1; i > 0 && LPCtypes[i].id != Pos; i--)
        /* nothing */;
        IspEnvironment->DetectedDevice = i;
    if (IspEnvironment->DetectedDevice == 0) {
        DebugPrintf(2, "unknown");
    }
    else {
        DebugPrintf(2, "LPC%s, %d kiB ROM / %d kiB SRAM",
            LPCtypes[IspEnvironment->DetectedDevice].Product,
            LPCtypes[IspEnvironment->DetectedDevice].FlashSize,
            LPCtypes[IspEnvironment->DetectedDevice].RAMSize);
    }
    DebugPrintf(2, " (0x%X)\n", Pos);//strippedAnswer);

    if (!IspEnvironment->DetectOnly)
    {
        // Build up uuencode table
        uuencode_table[0] = 0x60;           // 0x20 is translated to 0x60 !

        for (i = 1; i < 64; i++)
        {
            uuencode_table[i] = (char)(0x20 + i);
        }

        if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC2XXX)
        {
            // Patch 0x14, otherwise it is not running and jumps to boot mode

            ivt_CRC = 0;

            // Clear the vector at 0x14 so it doesn't affect the checksum:
            for (i = 0; i < 4; i++)
            {
                IspEnvironment->BinaryContent[i + 0x14] = 0;
            }

            // Calculate a native checksum of the little endian vector table:
            for (i = 0; i < (4 * 8);) {
                ivt_CRC += IspEnvironment->BinaryContent[i++];
                ivt_CRC += IspEnvironment->BinaryContent[i++] << 8;
                ivt_CRC += IspEnvironment->BinaryContent[i++] << 16;
                ivt_CRC += IspEnvironment->BinaryContent[i++] << 24;
            }

            /* Negate the result and place in the vector at 0x14 as little endian
            * again. The resulting vector table should checksum to 0. */
            ivt_CRC = (unsigned long) (0 - ivt_CRC);
            for (i = 0; i < 4; i++)
            {
                IspEnvironment->BinaryContent[i + 0x14] = (unsigned char)(ivt_CRC >> (8 * i));
            }

            DebugPrintf(3, "Position 0x14 patched: ivt_CRC = 0x%08lX\n", ivt_CRC);
        }
        else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC17XX ||
                LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC13XX ||
                LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC11XX)
        {
            // Patch 0x1C, otherwise it is not running and jumps to boot mode

            ivt_CRC = 0;

            // Clear the vector at 0x1C so it doesn't affect the checksum:
            for (i = 0; i < 4; i++)
            {
                IspEnvironment->BinaryContent[i + 0x1C] = 0;
            }

            // Calculate a native checksum of the little endian vector table:
            for (i = 0; i < (4 * 8);) {
                ivt_CRC += IspEnvironment->BinaryContent[i++];
                ivt_CRC += IspEnvironment->BinaryContent[i++] << 8;
                ivt_CRC += IspEnvironment->BinaryContent[i++] << 16;
                ivt_CRC += IspEnvironment->BinaryContent[i++] << 24;
            }

            /* Negate the result and place in the vector at 0x1C as little endian
            * again. The resulting vector table should checksum to 0. */
            ivt_CRC = (unsigned long) (0 - ivt_CRC);
            for (i = 0; i < 4; i++)
            {
                IspEnvironment->BinaryContent[i + 0x1C] = (unsigned char)(ivt_CRC >> (8 * i));
            }

            DebugPrintf(3, "Position 0x1C patched: ivt_CRC = 0x%08lX\n", ivt_CRC);
        }
        else
        {
          DebugPrintf(1, "Internal error: wrong chip variant %d (detected device %d)\n", LPCtypes[IspEnvironment->DetectedDevice].ChipVariant, IspEnvironment->DetectedDevice);
          exit(1);
        }
    }

#if 0
    DebugPrintf(2, "Read Unique ID:\n");

    cmdstr = "N\n";

    SendComPort(IspEnvironment, cmdstr);

    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 5,5000);

    if (strncmp(Answer, cmdstr, strlen(cmdstr)) != 0)
    {
        DebugPrintf(1, "no answer on Read Unique ID\n");
        return (NO_ANSWER_RBV);
    }

    if (strncmp(Answer + strlen(cmdstr), "0\r\n", 3) == 0)
    {
        strippedAnswer = Answer + strlen(cmdstr) + 3;
        DebugPrintf(2, strippedAnswer);
    }
    else
    {
        DebugPrintf(2, "unknown\n");
    }
#endif // 0

    /* In case of a download to RAM, use full RAM for downloading
    * set the flash parameters to full RAM also.
    * This makes sure that all code is downloaded as one big sector
    */

    if (IspEnvironment->BinaryOffset >= ReturnValueLpcRamStart(IspEnvironment))
    {
        LPCtypes[IspEnvironment->DetectedDevice].FlashSectors = 1;
        LPCtypes[IspEnvironment->DetectedDevice].MaxCopySize  = LPCtypes[IspEnvironment->DetectedDevice].RAMSize*1024 - (ReturnValueLpcRamBase(IspEnvironment) - ReturnValueLpcRamStart(IspEnvironment));
        LPCtypes[IspEnvironment->DetectedDevice].SectorTable  = SectorTable_RAM;
        SectorTable_RAM[0] = LPCtypes[IspEnvironment->DetectedDevice].MaxCopySize;
    }
    if (IspEnvironment->DetectOnly)
        return (0);


    // Start with sector 1 and go upward... Sector 0 containing the interrupt vectors
    // will be loaded last, since it contains a checksum and device will re-enter
    // bootloader mode as long as this checksum is invalid.
    DebugPrintf(2, "Will start programming at Sector 1 if possible, and conclude with Sector 0 to ensure that checksum is written last.\n");
    if (LPCtypes[IspEnvironment->DetectedDevice].SectorTable[0] >= IspEnvironment->BinaryLength)
    {
        Sector = 0;
        SectorStart = 0;
    }
    else
    {
        SectorStart = LPCtypes[IspEnvironment->DetectedDevice].SectorTable[0];
        Sector = 1;
    }

    if (IspEnvironment->WipeDevice == 1)
    {
        DebugPrintf(2, "Wiping Device. ");

        sprintf(tmpString, "P %d %d\n", 0, LPCtypes[IspEnvironment->DetectedDevice].FlashSectors-1);

        if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
        {
            DebugPrintf(1, "Wrong answer on Prepare-Command\n");
            return (WRONG_ANSWER_PREP + GetAndReportErrorNumber(Answer));
        }

        sprintf(tmpString, "E %d %d\n", 0, LPCtypes[IspEnvironment->DetectedDevice].FlashSectors-1);

        if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
        {
            DebugPrintf(1, "Wrong answer on Erase-Command\n");
            return (WRONG_ANSWER_ERAS + GetAndReportErrorNumber(Answer));
        }
        DebugPrintf(2, "OK \n");
    }
    else{
        //no wiping requested: erasing sector 0 first
        DebugPrintf(2, "Erasing sector 0 first, to invalidate checksum. ");

        sprintf(tmpString, "P %d %d\n", 0, 0);

        if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
        {
            DebugPrintf(1, "Wrong answer on Prepare-Command\n");
            return (WRONG_ANSWER_PREP + GetAndReportErrorNumber(Answer));
        }

        sprintf(tmpString, "E %d %d\n", 0, 0);

        if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
        {
            DebugPrintf(1, "Wrong answer on Erase-Command\n");
            return (WRONG_ANSWER_ERAS + GetAndReportErrorNumber(Answer));
        }
        DebugPrintf(2, "OK \n");
    }
    while (1)
    {
        if (Sector >= LPCtypes[IspEnvironment->DetectedDevice].FlashSectors)
        {
            DebugPrintf(1, "Program too large; running out of Flash sectors.\n");
            return (PROGRAM_TOO_LARGE);
        }

        DebugPrintf(2, "Sector %ld: ", Sector);
        fflush(stdout);

        if (IspEnvironment->BinaryOffset < ReturnValueLpcRamStart(IspEnvironment)) // Skip Erase when running from RAM
        {
            sprintf(tmpString, "P %ld %ld\n", Sector, Sector);

            if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
            {
                DebugPrintf(1, "Wrong answer on Prepare-Command (1) (Sector %ld)\n", Sector);
                return (WRONG_ANSWER_PREP + GetAndReportErrorNumber(Answer));
            }

            DebugPrintf(2, ".");
            fflush(stdout);
            if (IspEnvironment->WipeDevice == 0 && (Sector!=0)) //Sector 0 already erased
            {
                sprintf(tmpString, "E %ld %ld\n", Sector, Sector);

                if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
                {
                    DebugPrintf(1, "Wrong answer on Erase-Command (Sector %ld)\n", Sector);
                    return (WRONG_ANSWER_ERAS + GetAndReportErrorNumber(Answer));
                }

                DebugPrintf(2, ".");
                fflush(stdout);
            }
        }

        SectorLength = LPCtypes[IspEnvironment->DetectedDevice].SectorTable[Sector];
        if (SectorLength > IspEnvironment->BinaryLength - SectorStart)
        {
            SectorLength = IspEnvironment->BinaryLength - SectorStart;
        }

        for (SectorOffset = 0; SectorOffset < SectorLength; SectorOffset += SectorChunk)
        {
            if (SectorOffset > 0)
            {
                // Add a visible marker between segments in a sector
                DebugPrintf(2, "|");  /* means: partial segment copied */
                fflush(stdout);
            }

            // If the Flash ROM sector size is bigger than the number of bytes
            // we can copy from RAM to Flash, we must "chop up" the sector and
            // copy these individually.
            // This is especially needed in the case where a Flash sector is
            // bigger than the amount of SRAM.
            SectorChunk = SectorLength - SectorOffset;
            if (SectorChunk > (unsigned)LPCtypes[IspEnvironment->DetectedDevice].MaxCopySize)
            {
                SectorChunk = LPCtypes[IspEnvironment->DetectedDevice].MaxCopySize;
            }

            // Write multiple of 45 * 4 Byte blocks to RAM, but copy maximum of on sector to Flash
            // In worst case we transfer up to 180 byte to much to RAM
            // but then we can always use full 45 byte blocks and length is multiple of 4
            CopyLength = SectorChunk;
            if ((CopyLength % (45 * 4)) != 0)
            {
                CopyLength += ((45 * 4) - (CopyLength % (45 * 4)));
            }

            sprintf(tmpString, "W %ld %ld\n", ReturnValueLpcRamBase(IspEnvironment), CopyLength);

            if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
            {
                DebugPrintf(1, "Wrong answer on Write-Command\n");
                return (WRONG_ANSWER_WRIT + GetAndReportErrorNumber(Answer));
            }

            DebugPrintf(2, ".");
            fflush(stdout);

            block_CRC = 0;
            Line = 0;

            // Transfer blocks of 45 * 4 bytes to RAM
            for (Pos = SectorStart + SectorOffset; (Pos < SectorStart + SectorOffset + CopyLength) && (Pos < IspEnvironment->BinaryLength); Pos += (45 * 4))
            {
                for (Block = 0; Block < 4; Block++)  // Each block 45 bytes
                {
                    DebugPrintf(2, ".");
                    fflush(stdout);

#if defined INTEGRATED_IN_WIN_APP
                    // inform the calling application about having written another chuck of data
                    AppWritten(45);
#endif

                    // Uuencode one 45 byte block
                    tmpStringPos = 0;

#if !defined COMPILE_FOR_LPC21
                    sendbuf[Line][tmpStringPos++] = (char)(' ' + 45);    // Encode Length of block
#else
                    tmpString[tmpStringPos++] = (char)(' ' + 45);        // Encode Length of block
#endif

                    for (BlockOffset = 0; BlockOffset < 45; BlockOffset++)
                    {
                        if (IspEnvironment->BinaryOffset < ReturnValueLpcRamStart(IspEnvironment))
                        { // Flash: use full memory
                            c = IspEnvironment->BinaryContent[Pos + Block * 45 + BlockOffset];
                        }
                        else
                        { // RAM: Skip first 0x200 bytes, these are used by the download program in LPC21xx
                            c = IspEnvironment->BinaryContent[Pos + Block * 45 + BlockOffset + 0x200];
                        }

                        block_CRC += c;

                        k = (k << 8) + (c & 255);

                        if ((BlockOffset % 3) == 2)   // Collecting always 3 Bytes, then do processing in 4 Bytes
                        {
#if !defined COMPILE_FOR_LPC21
                            sendbuf[Line][tmpStringPos++] = uuencode_table[(k >> 18) & 63];
                            sendbuf[Line][tmpStringPos++] = uuencode_table[(k >> 12) & 63];
                            sendbuf[Line][tmpStringPos++] = uuencode_table[(k >>  6) & 63];
                            sendbuf[Line][tmpStringPos++] = uuencode_table[ k        & 63];
#else
                            tmpString[tmpStringPos++] = uuencode_table[(k >> 18) & 63];
                            tmpString[tmpStringPos++] = uuencode_table[(k >> 12) & 63];
                            tmpString[tmpStringPos++] = uuencode_table[(k >>  6) & 63];
                            tmpString[tmpStringPos++] = uuencode_table[ k        & 63];
#endif
                        }
                    }


#if !defined COMPILE_FOR_LPC21
                    sendbuf[Line][tmpStringPos++] = '\n';
                    sendbuf[Line][tmpStringPos++] = 0;

                    SendComPort(IspEnvironment, sendbuf[Line]);
                    // receive only for debug proposes
                    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 1,5000);
#else
                    tmpString[tmpStringPos++] = '\n';
                    tmpString[tmpStringPos++] = 0;

                    SendComPort(IspEnvironment, tmpString);
                    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 1,5000);
                    if (strncmp(Answer, tmpString, tmpStringPos) != 0)
                    {
                        DebugPrintf(1, "Error on writing data (1)\n");
                        return (ERROR_WRITE_DATA);
                    }
#endif

                    Line++;

                    DebugPrintf(3, "Line = %d\n", Line);

                    if (Line == 20)
                    {
#if !defined COMPILE_FOR_LPC21
                        for (repeat = 0; repeat < 3; repeat++)
                        {

                            // printf("block_CRC = %ld\n", block_CRC);

                            sprintf(tmpString, "%ld\n", block_CRC);

                            SendComPort(IspEnvironment, tmpString);

                            ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 2,5000);

                            sprintf(tmpString, "%ld\nOK\r\n", block_CRC);

                            if (strcmp(Answer, tmpString) != 0)
                            {
                                for (i = 0; i < Line; i++)
                                {
                                    SendComPort(IspEnvironment, sendbuf[i]);
                                    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 1,5000);
                                }
                            }
                            else
                                break;
                        }

                        if (repeat >= 3)
                        {
                            DebugPrintf(1, "Error on writing block_CRC (1)\n");
                            return (ERROR_WRITE_CRC);
                        }
#else
                        // printf("block_CRC = %ld\n", block_CRC);
                        sprintf(tmpString, "%ld\r\n", block_CRC);
                        SendComPort(IspEnvironment, tmpString);

                        ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 2,5000);

                        sprintf(tmpString, "%ld\r\nOK\r\n", block_CRC);
                        if (strcmp(Answer, tmpString) != 0)
                        {
                            DebugPrintf(1, "Error on writing block_CRC (2)\n");
                            return (ERROR_WRITE_CRC);
                        }
#endif
                        Line = 0;
                        block_CRC = 0;
                    }
                }
            }

            if (Line != 0)
            {
#if !defined COMPILE_FOR_LPC21
                for (repeat = 0; repeat < 3; repeat++)
                {
                    sprintf(tmpString, "%ld\n", block_CRC);

                    SendComPort(IspEnvironment, tmpString);

                    ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 2,5000);

                    sprintf(tmpString, "%ld\nOK\r\n", block_CRC);

                    if (strcmp(Answer, tmpString) != 0)
                    {
                        for (i = 0; i < Line; i++)
                        {
                            SendComPort(IspEnvironment, sendbuf[i]);
                            ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 1,5000);
                        }
                    }
                    else
                        break;
                }

                if (repeat >= 3)
                {
                    DebugPrintf(1, "Error on writing block_CRC (3)\n");
                    return (ERROR_WRITE_CRC2);
                }
#else
                sprintf(tmpString, "%ld\r\n", block_CRC);
                SendComPort(IspEnvironment, tmpString);

                ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 2,5000);

                sprintf(tmpString, "%ld\r\nOK\r\n", block_CRC);
                if (strcmp(Answer, tmpString) != 0)
                {
                    DebugPrintf(1, "Error on writing block_CRC (4)\n");
                    return (ERROR_WRITE_CRC2);
                }
#endif
            }

            if (IspEnvironment->BinaryOffset < ReturnValueLpcRamStart(IspEnvironment))
            {
                // Prepare command must be repeated before every write
                sprintf(tmpString, "P %ld %ld\n", Sector, Sector);

                if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
                {
                    DebugPrintf(1, "Wrong answer on Prepare-Command (2) (Sector %ld)\n", Sector);
                    return (WRONG_ANSWER_PREP2 + GetAndReportErrorNumber(Answer));
                }

                // Round CopyLength up to one of the following values: 512, 1024,
                // 4096, 8192; but do not exceed the maximum copy size (usually
                // 8192, but chip-dependent)
                if (CopyLength < 512)
                {
                    CopyLength = 512;
                }
                else if (SectorLength < 1024)
                {
                    CopyLength = 1024;
                }
                else if (SectorLength < 4096)
                {
                    CopyLength = 4096;
                }
                else
                {
                    CopyLength = 8192;
                }
                if (CopyLength > (unsigned)LPCtypes[IspEnvironment->DetectedDevice].MaxCopySize)
                {
                    CopyLength = LPCtypes[IspEnvironment->DetectedDevice].MaxCopySize;
                }

                sprintf(tmpString, "C %ld %ld %ld\n", SectorStart + SectorOffset, ReturnValueLpcRamBase(IspEnvironment), CopyLength);

                if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
                {
                    DebugPrintf(1, "Wrong answer on Copy-Command\n");
                    return (WRONG_ANSWER_COPY + GetAndReportErrorNumber(Answer));
                }

                if (IspEnvironment->Verify)
                {

                    //Avoid compare first 64 bytes.
                    //Because first 64 bytes are re-mapped to flash boot sector,
                    //and the compare result may not be correct.
                    if (SectorStart + SectorOffset<64)
                    {
                        sprintf(tmpString, "M %d %ld %ld\n", 64, ReturnValueLpcRamBase(IspEnvironment) + (64 - SectorStart - SectorOffset), CopyLength-(64 - SectorStart - SectorOffset));
                    }
                    else
                    {
                        sprintf(tmpString, "M %ld %ld %ld\n", SectorStart + SectorOffset, ReturnValueLpcRamBase(IspEnvironment), CopyLength);
                    }

                    if (!SendAndVerify(IspEnvironment, tmpString, Answer, sizeof Answer))
                    {
                        DebugPrintf(1, "Wrong answer on Compare-Command\n");
                        return (WRONG_ANSWER_COPY + GetAndReportErrorNumber(Answer));
                    }
                }
            }
        }

        DebugPrintf(2, "\n");
        fflush(stdout);

        if ((SectorStart + SectorLength) >= IspEnvironment->BinaryLength && Sector!=0)
        {
            Sector = 0;
            SectorStart = 0;
        }
        else if (Sector == 0) {
            break;
        }
        else {
            SectorStart += LPCtypes[IspEnvironment->DetectedDevice].SectorTable[Sector];
            Sector++;
        }
    }

    tDoneUpload = time(NULL);
    if (IspEnvironment->Verify)
        DebugPrintf(2, "Download Finished and Verified correct... taking %d seconds\n", tDoneUpload - tStartUpload);
    else
        DebugPrintf(2, "Download Finished... taking %d seconds\n", tDoneUpload - tStartUpload);

    if(IspEnvironment->DoNotStart == 0)
    {
        DebugPrintf(2, "Now launching the brand new code\n");
        fflush(stdout);

        if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC2XXX)
        {
            sprintf(tmpString, "G %ld A\n", IspEnvironment->StartAddress);
        }
        else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC17XX ||
                LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC13XX ||
                LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC11XX)
        {
            sprintf(tmpString, "G %ld T\n", IspEnvironment->StartAddress & ~1);
        }
        else
        {
            printf("Internal Error %s %d\n", __FILE__, __LINE__);
            exit(1);
        }

        SendComPort(IspEnvironment, tmpString); //goto 0 : run this fresh new downloaded code code
        if (IspEnvironment->BinaryOffset < ReturnValueLpcRamStart(IspEnvironment))
        { // Skip response on G command - show response on Terminal instead
            ReceiveComPort(IspEnvironment, Answer, sizeof(Answer)-1, &realsize, 2, 5000);
            /* the reply string is frequently terminated with a -1 (EOF) because the
            * connection gets broken; zero-terminate the string ourselves
            */
            while (realsize > 0 && ((signed char) Answer[(int)realsize - 1]) < 0)
                realsize--;
            Answer[(int)realsize] = '\0';
            /* Better to check only the first 9 chars instead of complete receive buffer,
            * because the answer can contain the output by the started programm
            */
            if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC2XXX)
            {
                sprintf(ExpectedAnswer, "G %ld A\n0", IspEnvironment->StartAddress);
            }
            else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC17XX ||
                    LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC13XX ||
                    LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC11XX)
            {
                sprintf(ExpectedAnswer, "G %ld T\n0", IspEnvironment->StartAddress & ~1);
            }
            else
            {
                printf("Internal Error %s %d\n", __FILE__, __LINE__);
                exit(1);
            }

            if (realsize == 0 || strncmp((const char *)Answer, /*cmdstr*/ExpectedAnswer, strlen(/*cmdstr*/ExpectedAnswer)) != 0)
            {
                DebugPrintf(2, "Failed to run the new downloaded code: ");
                return (FAILED_RUN + GetAndReportErrorNumber(Answer));
            }
        }

        fflush(stdout);
    }
    return (0);
}
#endif // LPC_SUPPORT


unsigned long ReturnValueLpcRamStart(ISP_ENVIRONMENT *IspEnvironment)
{
  if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC2XXX)
  {
    return LPC_RAMSTART_LPC2XXX;
  }
  else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC17XX)
  {
    return LPC_RAMSTART_LPC17XX;
  }
  else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC13XX)
  {
    return LPC_RAMSTART_LPC13XX;
  }
  else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC11XX)
  {
    return LPC_RAMSTART_LPC11XX;
  }
  printf("Error in ReturnValueLpcRamStart (%d)\n", LPCtypes[IspEnvironment->DetectedDevice].ChipVariant);
  exit(1);
}


unsigned long ReturnValueLpcRamBase(ISP_ENVIRONMENT *IspEnvironment)
{
  if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC2XXX)
  {
    return LPC_RAMBASE_LPC2XXX;
  }
  else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC17XX)
  {
    return LPC_RAMBASE_LPC17XX;
  }
  else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC13XX)
  {
    return LPC_RAMBASE_LPC13XX;
  }
  else if(LPCtypes[IspEnvironment->DetectedDevice].ChipVariant == CHIP_VARIANT_LPC11XX)
  {
    return LPC_RAMBASE_LPC11XX;
  }
  printf("Error in ReturnValueLpcRamBase (%d)\n", LPCtypes[IspEnvironment->DetectedDevice].ChipVariant);
  exit(1);
}
