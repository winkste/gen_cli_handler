/****************************************************************************************
* FILENAME :        main.c
*
* SHORT DESCRIPTION:
*   CLI basics using argtable3 and embedded_cli libraries. 
*
* DETAILED DESCRIPTION : --- 
*
* AUTHOR :    Stephan Wink        CREATED ON :    03. Mar. 2024
*
* Copyright (c) [2020] [Stephan Wink]
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
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
****************************************************************************************/

/***************************************************************************************/
/* Include Interfaces */
#include <unistd.h>
#include <termios.h>

#include "wsconsole.h"
#include "wserr.h"
#include "argtable3.h"

/***************************************************************************************/
/* Local constant defines */

/***************************************************************************************/
/* Local function like makros */

/***************************************************************************************/
/* Local type definitions (enum, struct, union) */

/***************************************************************************************/
/* Local functions prototypes: */
static char GetCharacter_c(void);
static void InterruptHandler_vd(int dummy);
static void PosixPutCharacter_vd(void *data_vp, char character_c, bool isLastChar_b);
static wserr_t AddCommand_t(wsconsole_cmdItem_tp cmd_pt, FILE *resp_fp);

/***************************************************************************************/
/* Local variables: */
/**
 * Object of the console.
 */
static wsconsole_tp console_xs;

/***************************************************************************************/
/* Global functions (unlimited visibility) */
/**--------------------------------------------------------------------------------------
 * @brief     Main function and entry point for executable
 * @author    S. Wink
 * @date      03. Mar. 2024
*//*-----------------------------------------------------------------------------------*/
int main(void)
{
    wsconsole_config_t consoleConfig_sts;
    wsconsole_cmdItem_t command_st;

    wserr_LOG(wsconsole_InitParameter_t(&consoleConfig_sts));
    consoleConfig_sts.getCharFunc_fp = GetCharacter_c;
    consoleConfig_sts.intHandler_fp = InterruptHandler_vd;
    consoleConfig_sts.putCharFunc_fp = PosixPutCharacter_vd;

    console_xs = wsconsole_AllocateConsole_t();
    wserr_LOG(wsconsole_Init_t(console_xs, &consoleConfig_sts));
      
    command_st.command = "add";
    command_st.hint = NULL;
    command_st.help = "Adds the two numbers and returns the result.";
    command_st.func = &AddCommand_t;
    struct arg_int *add_args[2];
    add_args[0] = arg_int0(NULL, NULL, "<a>", "First number");
    add_args[1] = arg_int0(NULL, NULL, "<b>", "Second number");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {add_args[0], add_args[1], end};
    command_st.argtable = argtable;

    wserr_LOG(wsconsole_RegisterCommand_t(console_xs, &command_st));

    while(true)
    {
        wserr_LOG(wsconsole_Run_t(console_xs));
    }
 
    wserr_LOG(wsconsole_DeInit_t(console_xs));   
}

/***************************************************************************************/
/* Local functions: */

/**--------------------------------------------------------------------------------------
 * @brief     This function retrieves exactly one character from stdin, in 
 *            character-by-character mode (as opposed to reading a full line)
 * @author    S. Wink
 * @date      03. Mar. 2024
 * @return    one character retrieved by stdin
*//*-----------------------------------------------------------------------------------*/
static char GetCharacter_c(void)
{
    char buffer_c = 0;
    struct termios old_st = {0};
    if (tcgetattr(0, &old_st) < 0)
        perror("tcsetattr()");

    struct termios raw = old_st;

    /* Do what cfmakeraw does (Using --std=c99 means that cfmakeraw isn't
    available) */
    raw.c_iflag &=
        ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag &= ~OPOST;
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &raw) < 0)
        perror("tcsetattr ICANON");

    if (read(0, &buffer_c, 1) < 0)
        perror("read()");

    if (tcsetattr(0, TCSADRAIN, &old_st) < 0)
        perror("tcsetattr ~ICANON");
    return (buffer_c);
}

/**--------------------------------------------------------------------------------------
 * @brief     The interrupt handler function is a signal handler for SIGINT (Ctrl-C). 
 *            It's used to capture Ctrl-C and interrupt the CLI input.
 * @author    S. Wink
 * @date      03. Mar. 2024
 * @param     dummy             input data pointer
 * @return    none
*//*-----------------------------------------------------------------------------------*/
static void InterruptHandler_vd(int dummy)
{
    /*Not working on the macbook*/
    (void)dummy;
    printf("Ctrl-C received!\n");
}

/**--------------------------------------------------------------------------------------
 * @brief     This function outputs a single character to stdout, to be used as the
 *            callback from embedded cli.
 * @author    S. Wink
 * @date      03. Mar. 2024
 * @param     *data_vp      pointer to data channel
 * @param     character_c   character to print to channel
 * @param     isLastChar_b  is this the last character, than flush the complete message
 * @return    none
*//*-----------------------------------------------------------------------------------*/
static void PosixPutCharacter_vd(void *data_vp, char character_c, bool isLastChar_b)
{
    FILE *outStream_fp = data_vp;

    fputc(character_c, outStream_fp);
    if(isLastChar_b)
        fflush(outStream_fp);
}

/**--------------------------------------------------------------------------------------
 * @brief     This is the callback function to add two numbers.
 * @author    S. Wink
 * @date      14. Mar. 2024
 * @param     cmd_pt     pointer to command
 * @param     resp_fp    FILE object to create the response
 * @return    wserr_OK
*//*-----------------------------------------------------------------------------------*/
static wserr_t AddCommand_t(wsconsole_cmdItem_tp cmd_pt, FILE *resp_fp)
{
    int lineFeed_i = 0;
    struct arg_int* *argtable_ppst;
    struct arg_int *arg1_ptr;
    struct arg_int *arg2_ptr;
    int result;

    argtable_ppst = (struct arg_int**) cmd_pt->argtable;
    arg1_ptr = *argtable_ppst;
    argtable_ppst++;
    arg2_ptr = *argtable_ppst;

    result = *arg1_ptr->ival + *arg2_ptr->ival;

    fprintf(resp_fp, "The result is: %d\n", result);
    return wserr_OK;
}
