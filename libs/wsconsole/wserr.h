/*****************************************************************************************
* FILENAME :        wsconsole.h
*
* DESCRIPTION :
*       Header file for console parsing independent on the input channel
*
* Date: 04. MAr 2024
*
* NOTES :
*
* Copyright (c) [2024] [Stephan Wink]
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
vAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*****************************************************************************************/
#ifndef WSERR_H
#define WSERR_H

#ifdef __cplusplus
extern "C"
{
#endif
/****************************************************************************************/
/* Imported header files: */

#include <stddef.h>
#include <stdint.h>

/****************************************************************************************/
/* Global constant defines: */
#define wserr_OK                    0x0000
#define wserr_ERR_GEN               0x0001
#define wserr_ERR_NO_MEM            0x0002
#define wserr_ERR_INVALID_STATE     0x0003
#define wserr_ERR_PARAM             0x0004
/****************************************************************************************/
/* Global function like macro defines (to be avoided): */

#define wserr_LOG(X)X;

/****************************************************************************************/
/* Global type definitions (enum (en), struct (st), union (un), typedef (tx): */

typedef int wserr_t;

/****************************************************************************************/
/* Global function definitions: */

/****************************************************************************************/
/* Global data definitions: */

#ifdef __cplusplus
}
#endif

#endif //WSERR