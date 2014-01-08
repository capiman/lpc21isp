/******************************************************************************

Project:           Portable command line ISP for NXP LPC1000 / LPC2000 family
                   and Analog Devices ADUC70xx

Filename:          adprog.c

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

#if defined(_WIN32)
#if !defined __BORLANDC__
#include "StdAfx.h"
#endif
#endif // defined(_WIN32)
#include "lpc21isp.h"

#ifdef AD_SUPPORT
#include "adprog.h"

/***************************** AnalogDevicesSync ************************/
/**  Attempt to synchronize with an Analog Device ARM micro.  Sends a
backspace and reads back the microcontrollers response.  Performs
multiple retries. Exits the program on error, returns to caller in the
case of success.
*/
static void AnalogDevicesSync(ISP_ENVIRONMENT *IspEnvironment)
{
    BINARY sync;                        /* Holds sync command.          */
    AD_SYNC_RESPONSE response;          /* Response from micro.         */
    int sync_attempts;                  /* Number of retries.           */

    /*  Make sure we don't read garbage later instead of the        */
    /* response we expect from the micro.                           */
    ClearSerialPortBuffers(IspEnvironment);

    DebugPrintf(2, "Synchronizing\n"); /* Progress report.             */

    sync = ANALOG_DEVICES_SYNC_CHAR;    /* Build up sync command.       */

    /*  Perform the actual sync attempt.  First send the sync       */
    /* character, the attempt to read back the response.  For the   */
    /* AD ARM micro this is a fixed length block.  If response is   */
    /* received attempt to validate it by comparing the first       */
    /* characters to those expected.  If the received block does    */
    /* not validate or is incomplete empty the serial buffer and    */
    /* retry.                                                       */
    for (sync_attempts = 0; sync_attempts < 5; sync_attempts++)
    {
        SendComPortBlock(IspEnvironment, &sync, 1);

        if (ReceiveComPortBlockComplete(IspEnvironment, &response, sizeof(response),
            500) == 0)
        {

            if (memcmp(response.product_id, ANALOG_DEVICES_SYNC_RESPONSE,
                ANALOG_DEVICES_SYNC_SIZE) == 0)
            {
                return;
            }
            else
            {
                DumpString(3, &response, sizeof(response),
                    "Unexpected response to sync attempt ");
            }
        }
        else
        {
            DebugPrintf(3, "No (or incomplete) answer on sync attempt\n");
        }

        ClearSerialPortBuffers(IspEnvironment);
    }

    DebugPrintf(1, "No (or unacceptable) answer on sync attempt\n");
    exit(4);
}

typedef struct {
    char start1;
    char start2;
    BINARY bytes;
    char cmd;
    BINARY address_h;
    BINARY address_u;
    BINARY address_m;
    BINARY address_l;
    BINARY data[251];
} AD_PACKET;

/***************************** AnalogDevicesFormPacket ******************/
/**  Create an Analog Devices communication packet from the constituent
elements.
\param [in] cmd The command being sent, one of 'E' for erase, 'W' for
write, 'V' for verify or 'R' for run..
\param [in] no_bytes the number of data bytes to send with the command in
the packet.
\param [in] address the address to apply the command to.
\param [in] data the data to send with the packet, may be null if no_bytes
is zero.
\param[out] packet that will be filled.
*/
static void AnalogDevicesFormPacket(ISP_ENVIRONMENT *IspEnvironment,
                                                char cmd, int no_bytes, unsigned int address,
                                                const void *data, AD_PACKET *packet)
{
    BINARY checksum;
    const BINARY *data_in;
    int i;

    (void)IspEnvironment; /* never used in this function */

    /*  Some sanity checking on the arguments.  These should only   */
    /* fail if there is a bug in the caller.                        */
    /*  Check 1) that the number of data bytes is in an acceptable  */
    /* range, 2) that we have a non-null pointer if data is being   */
    /* put in the packet and 3) that we have a non-null pointer to  */
    /* the packet to be filled. We just exit with an error message  */
    /* if any of these tests fail.                                  */
    if ((no_bytes < 0) || (no_bytes > 250))
    {
        DebugPrintf(1,
            "The number of bytes (%d) passed to FormPacket is invalid.\n",
            no_bytes);
        exit(-1);
    }
    if ((data == 0) && (no_bytes != 0))
    {
        DebugPrintf(1,
            "A null pointer to data paased to FormPacket when data was expected.\n");
        exit(-1);
    }
    if (packet == 0)
    {
        DebugPrintf(1,
            "A null packet pointer was passed to FormPacket.\n");
        exit(-1);
    }

    checksum = 0;               /*  Checksum starts at zero.            */

    data_in = (BINARY*) data;             /*  Pointer pun so we can walk through  */
    /* the data.                            */

    packet->start1 = 0x7;       /*  The start of the packet is constant.*/
    packet->start2 = 0xE;

    /*  Fill in the rest of the packet and calculate the checksum   */
    /* as we go.                                                    */

    /* The number of bytes is the number of data bytes + the        */
    /* address bytes + the command byte.                            */
    packet->bytes = (BINARY)(no_bytes + 5);

    checksum += packet->bytes;

    /*  The command for the packet being sent.  No error checking   */
    /* done on this.                                                */
    packet->cmd = cmd;

    checksum += cmd;

    /*  Now break up the address and place in the proper packet     */
    /* locations.                                                   */
    packet->address_l = (BINARY)(address & 0xFF);
    packet->address_m = (BINARY)((address >> 8) & 0xFF);
    packet->address_u = (BINARY)((address >> 16) & 0xFF);
    packet->address_h = (BINARY)((address >> 24) & 0xFF);

    checksum += packet->address_l;
    checksum += packet->address_m;
    checksum += packet->address_u;
    checksum += packet->address_h;

    /*  Copy the data bytes into the packet.  We could use memcpy   */
    /* but we have to calculate the checksum anyway.                */
    for (i = 0; i < no_bytes; i++)
    {
        packet->data[i] = data_in[i];
        checksum += data_in[i];
    }

    /*  Finally, add the checksum to the end of the packet.         */
    packet->data[i] = (BINARY)-checksum;
}

/***************************** AnalogDevicesSendPacket ******************/
/**  Send a previously form Analog Devices communication.  Retry a
couple of times if needed but fail by exiting the program if no ACK is
forthcoming.
\param [in] packet the packet to send.
*/
static void AnalogDevicesSendPacket(ISP_ENVIRONMENT *IspEnvironment,
                                                const AD_PACKET * packet)
{
    BINARY response;
    int retry = 0;

    do {
        retry++;

        /*  Make sure we don't read garbage later instead of    */
        /* the response we expect from the micro.               */
        ClearSerialPortBuffers(IspEnvironment);

        /*  Send the packet, the size is the number of data     */
        /* bytes in the packet plus 3 bytes worth of header     */
        /* plus checksum.                                       */
        SendComPortBlock(IspEnvironment, packet, packet->bytes + 4);

        /*  Receive the response and check, return to caller    */
        /* if successful.                                       */
        if (ReceiveComPortBlockComplete(IspEnvironment, &response, 1, 5000) == 0)
        {
            if (response == ANALOG_DEVICES_ACK)
            {
                DebugPrintf(3, "Packet Sent\n");
                return;
            }
            if (response != ANALOG_DEVICES_NAK)
            {
                DebugPrintf(3, "Unexpected response to packet (%x)\n", (int)response);
            }
            DebugPrintf(2, "*");
        }
    } while (retry < 3);

    DebugPrintf(1, "Send packet failed\n");
    exit(-1);
}

/***************************** AnalogDevicesErase ***********************/
/**  Erase the Analog Devices micro.  We take the simple way out and
just erase the whole thing.
*/
static void AnalogDevicesErase(ISP_ENVIRONMENT *IspEnvironment)
{
    BINARY pages;
    AD_PACKET packet;

    pages = 0;
    DebugPrintf(2, "Erasing .. ");
    AnalogDevicesFormPacket(IspEnvironment, 'E', 1, 0, &pages, &packet);
    AnalogDevicesSendPacket(IspEnvironment, &packet);
    DebugPrintf(2, "Erased\n");
}

#define AD_PACKET_SIZE (250)

/***************************** AnalogDevicesWrite ***********************/
/**  Write the program.
\param [in] data the program to download to the micro.
\param [in] address where to start placing the program.
\param [in] bytes the size of the progrm to download.
*/
static void AnalogDevicesWrite(ISP_ENVIRONMENT *IspEnvironment,
                                         const void *data, long address, size_t bytes)
{
    AD_PACKET packet;
    const BINARY *prog_data;

    DebugPrintf(2, "Writing %d bytes ", bytes);
    prog_data = (const BINARY*) data;
    while (bytes > AD_PACKET_SIZE)
    {
        AnalogDevicesFormPacket(IspEnvironment, 'W', AD_PACKET_SIZE, address, prog_data, &packet);
        AnalogDevicesSendPacket(IspEnvironment, &packet);
        address += AD_PACKET_SIZE;
        prog_data += AD_PACKET_SIZE;
        bytes -= AD_PACKET_SIZE;
        DebugPrintf(2, ".");
    }
    if (bytes > 0)
    {
        AnalogDevicesFormPacket(IspEnvironment, 'W', bytes, address, prog_data, &packet);
        AnalogDevicesSendPacket(IspEnvironment, &packet);
        DebugPrintf(2, ".");
    }
}

/***************************** AnalogDevicesDownload ********************/
/**  Perform the download into an Analog Devices micro.  As a quick and
* dirty hack against flash relocations at 0x80000
* \return 0 if ok, error code else
* \ToDo: possible to implement the return value instead of calling
* exit() in sub-functions
*/
int AnalogDevicesDownload(ISP_ENVIRONMENT *IspEnvironment)
{
    AnalogDevicesSync(IspEnvironment);
    AnalogDevicesErase(IspEnvironment);
    if (IspEnvironment->BinaryLength > 0x80000)
    {
        DebugPrintf(2, "Note:  Flash remapped 0x80000 to 0.\n");
        AnalogDevicesWrite(IspEnvironment, IspEnvironment->BinaryContent + 0x80000, 0, IspEnvironment->BinaryLength-0x80000);
    }
    else
    {
        AnalogDevicesWrite(IspEnvironment, IspEnvironment->BinaryContent, 0, IspEnvironment->BinaryLength);
    }
    return (0);
}
#endif // AD_SUPPORT
