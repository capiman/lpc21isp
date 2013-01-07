/******************************************************************************

Project:           Portable command line ISP for NXP LPC1000 / LPC2000 family
                   and Analog Devices ADUC70xx

Filename:          lsp21isp.h

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

// #define INTEGRATED_IN_WIN_APP

#if defined(_WIN32) && !defined(__CYGWIN__)
#define COMPILE_FOR_WINDOWS
#define COMPILED_FOR "Windows"
#elif defined(__CYGWIN__)
#define COMPILE_FOR_CYGWIN
#define COMPILED_FOR "Cygwin"
#elif (defined(__arm__) || defined(__thumb__)) && !defined(__linux__)
#define COMPILE_FOR_LPC21
#define COMPILED_FOR "ARM"
#define printf iprintf
#elif defined(__APPLE__)
#define COMPILE_FOR_LINUX
#define COMPILED_FOR "Apple MacOS X"
#elif defined(__FreeBSD__)
#define COMPILE_FOR_LINUX
#define COMPILED_FOR "FreeBSD"
#elif defined(__OpenBSD__)
#define COMPILE_FOR_LINUX
#define COMPILED_FOR "OpenBSD"
#else
#define COMPILE_FOR_LINUX
#define COMPILED_FOR "Linux"
#endif

// The Required features can be enabled / disabled here
#define LPC_SUPPORT

#ifndef COMPILE_FOR_LPC21
#define AD_SUPPORT
#define TERMINAL_SUPPORT
#endif

#if defined COMPILE_FOR_WINDOWS || defined COMPILE_FOR_CYGWIN
#include <windows.h>
#include <io.h>
#endif // defined COMPILE_FOR_WINDOWS || defined COMPILE_FOR_CYGWIN

#if defined COMPILE_FOR_WINDOWS
#include <conio.h>
//#define TRACE(x) OutputDebugString(x)
#define TRACE(x) printf("%s",x)
#endif // defined COMPILE_FOR_WINDOWS

#if defined COMPILE_FOR_CYGWIN
//#define TRACE(x) OutputDebugString(x)
#define TRACE(x) printf("%s",x)
#endif // defined COMPILE_FOR_WINDOWS

#if defined COMPILE_FOR_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
extern void Sleep(unsigned long MilliSeconds);
#define TRACE(x) printf("%s",x)
#endif // defined COMPILE_FOR_LINUX

#if defined COMPILE_FOR_LINUX || defined COMPILE_FOR_CYGWIN
#include <termios.h>
#include <unistd.h>     // for read and return value of lseek
#include <sys/time.h>   // for select_time
extern int kbhit(void);
extern int getch(void);
extern struct termios keyboard_origtty;
#endif // defined COMPILE_FOR_LINUX || defined COMPILE_FOR_CYGWIN

#include <ctype.h>      // isdigit()
#include <stdio.h>      // stdout
#include <stdarg.h>
#include <time.h>
#if defined (COMPILE_FOR_LINUX)
#if defined(__OpenBSD__)
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#endif

#if defined COMPILE_FOR_LPC21
#include <stdlib.h>
#include <string.h>
//#include <lpc_ioctl.h>  // if using libc serial port communication
#else
#include <fcntl.h>
#endif

typedef enum
{
    NXP_ARM,
    ANALOG_DEVICES_ARM
} TARGET;

typedef enum
{
    PROGRAM_MODE,
    RUN_MODE
} TARGET_MODE;

typedef enum
{
    FORMAT_BINARY,
    FORMAT_HEX
} FILE_FORMAT_TYPE;

typedef unsigned char BINARY;               // Data type used for microcontroller

/** Used to create list of files to read in. */
typedef struct file_list FILE_LIST;

/** Structure used to build list of input files. */
struct file_list
{
    const char *name;       /**< The name of the input file.	*/
    FILE_LIST *prev;        /**< The previous file name in the list.*/
    char hex_flag;          /**< True if the input file is hex.	*/
};

typedef struct
{
#if !defined COMPILE_FOR_LPC21
    TARGET micro;                                // The type of micro that will be programmed.
    FILE_FORMAT_TYPE FileFormat;
    unsigned char ProgramChip;                // Normally set

    unsigned char ControlLines;
    unsigned char ControlLinesSwapped;
    unsigned char ControlLinesInverted;
    unsigned char LogFile;
    FILE_LIST *f_list;                  // List of files to read in.
    int nQuestionMarks; // how many times to try to synchronise
    int DoNotStart;
    char *serial_port;                  // Name of the serial port to use to
                                        // communicate with the microcontroller.
                                        // Read from the command line.
#endif // !defined COMPILE_FOR_LPC21

    unsigned char TerminalOnly;         // Declared here for lazyness saves ifdef's
#ifdef TERMINAL_SUPPORT
    unsigned char TerminalAfterUpload;
    unsigned char LocalEcho;
#endif

    unsigned char HalfDuplex;           // Only used for LPC Programming
    unsigned char DetectOnly;
    unsigned char WipeDevice;
    unsigned char Verify;
    int           DetectedDevice;       /* index in LPCtypes[] array */
    char *baud_rate;                    /**< Baud rate to use on the serial
                                           * port communicating with the
                                           * microcontroller. Read from the
                                           * command line.                        */

    char StringOscillator[6];           /**< Holds representation of oscillator
                                           * speed from the command line.         */

    BINARY *FileContent;
    BINARY *BinaryContent;              /**< Binary image of the                  */
                                          /* microcontroller's memory.            */
    unsigned long BinaryLength;
    unsigned long BinaryOffset;
    unsigned long StartAddress;
    unsigned long BinaryMemSize;

#if defined COMPILE_FOR_WINDOWS || defined COMPILE_FOR_CYGWIN
    HANDLE hCom;
#endif // defined COMPILE_FOR_WINDOWS || defined COMPILE_FOR_CYGWIN

#if defined COMPILE_FOR_LINUX || defined COMPILE_FOR_LPC21
    int fdCom;
#endif // defined COMPILE_FOR_LINUX || defined COMPILE_FOR_LPC21

#if defined COMPILE_FOR_LINUX
    struct termios oldtio, newtio;
#endif // defined COMPILE_FOR_LINUX

#ifdef INTEGRATED_IN_WIN_APP
    unsigned char NoSync;
#endif

    unsigned serial_timeout_count;   /**< Local used to track timeouts on serial port read. */

} ISP_ENVIRONMENT;

#if defined COMPILE_FOR_LPC21

#define DebugPrintf(in, ...)

#else
extern int debug_level;

#if defined INTEGRATED_IN_WIN_APP

#define DebugPrintf AppDebugPrintf
void AppDebugPrintf(int level, const char *fmt, ...);

#define exit(val)   AppException(val)
void AppException(int exception_level);

int AppDoProgram(int argc, char *argv[]);

#define Exclude_kbhit 1
int AppSyncing(int trials);
void AppWritten(int size);

#else
void DebugPrintf(int level, const char *fmt, ...);
//#define DebugPrintf(level, ...) if (level <= debug_level) { TRACE( __VA_ARGS__ ); }
#endif

void ClearSerialPortBuffers(ISP_ENVIRONMENT *IspEnvironment);

#endif


#if defined COMPILE_FOR_LINUX
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif // defined COMPILE_FOR_LINUX

#ifndef O_BINARY
#define O_BINARY 0
#endif // O_BINARY

#ifndef DWORD
#define DWORD unsigned long
#endif // DWORD

/*
debug levels
0 - very quiet          - Nothing gets printed at this level
1 - quiet               - Only error messages should be printed
2 - indicate progress   - Add progress messages
3 - first level debug   - Major level tracing
4 - second level debug  - Add detailed debugging
5 - log comm's          - log serial I/O
*/


void ReceiveComPort(ISP_ENVIRONMENT *IspEnvironment,
                    const char *Ans, unsigned long MaxSize,
                    unsigned long *RealSize, unsigned long WantedNr0x0A,
                    unsigned timeOutMilliseconds);
void PrepareKeyboardTtySettings(void);
void ResetKeyboardTtySettings(void);
void ResetTarget(ISP_ENVIRONMENT *IspEnvironment, TARGET_MODE mode);

void DumpString(int level, const void *s, size_t size, const char *prefix_string);
void SendComPort(ISP_ENVIRONMENT *IspEnvironment, const char *s);
void SendComPortBlock(ISP_ENVIRONMENT *IspEnvironment, const void *s, size_t n);
int ReceiveComPortBlockComplete(ISP_ENVIRONMENT *IspEnvironment, void *block, size_t size, unsigned timeout);
void ClearSerialPortBuffers(ISP_ENVIRONMENT *IspEnvironment);

int lpctest(char* FileName);
