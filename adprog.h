/******************************************************************************

Project:           Portable command line ISP for NXP LPC1000 / LPC2000 family
                   and Analog Devices ADUC70xx

Filename:          adprog.h

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

#define ANALOG_DEVICES_SYNC_CHAR                ((BINARY)0x08)
#define ANALOG_DEVICES_SYNC_RESPONSE        ("ADuC")
#define ANALOG_DEVICES_SYNC_SIZE                (strlen(ANALOG_DEVICES_SYNC_RESPONSE))
#define ANALOG_DEVICES_ACK                        0x6
#define ANALOG_DEVICES_NAK                        0x7

typedef struct
{
    BINARY product_id[15];
    BINARY version[3];
    BINARY reserved[4];
    BINARY terminator[2];
} AD_SYNC_RESPONSE;

int AnalogDevicesDownload(ISP_ENVIRONMENT *IspEnvironment);
