/*****************************************************************************************
* FILENAME :        wsconsole.h
*
* DESCRIPTION :
*       Header file for console parsing independent on the input channel
*
* Date: 04. MAr 2024
*
* NOTES :
* Functional flow description / User interface
*   wsconsole_tp console_x;
*   wsconsole_config_t consoleConfig_st;
*
*   wserr_LOG(wsconsole_InitParameter_t(&consoleConfig_st));
*   console_x = wsconsole_Allocate_t();
*   wserr_LOG(wsconsole_Init_t(wsconsole_tp console_x, consoleConfig_st));
*
*   wsconsole_cmd_t command_st;
*   
*   command_st.command = "help",
*   command_st.help = "Print the list of registered commands",
*   command_st.func = &HelpCommand_t
*   command_st.argtable = NULL;
*   wserr_LOG() myConsole_RegisterCommand_t(console_x, &command_st);
*   
*   command_st.command = "add",
*   command_st.help = "Adds the two numbers and returns the result."
*   command_st.func = &AddCommand_t
*   struct arg_int *add_args[2];
*   add_args[0] = arg_int0(NULL, NULL, "<a>", "First number");
*   add_args[1] = arg_int0(NULL, NULL, "<b>", "Second number");
*   struct arg_end *end = arg_end(20);
*   void *argtable[] = {add_args[0], add_args[1], end};
*   wserr_LOG(myConsole_RegisterCommand_t(console_x, &command_st));
*
*   while(true)
*   {
*       wserr_LOG(wsconsole_Run_td(console_x));  
*       break;
*   }
*
*   wserr_LOG(wsconsole_DeInit_t(console_x));
*   
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
#ifndef WSCONSOLE_H
#define WSCONSOLE_H

#ifdef __cplusplus
extern "C"
{
#endif
/****************************************************************************************/
/* Imported header files: */

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

#include "wserr.h"


/****************************************************************************************/
/* Global constant defines: */

/****************************************************************************************/
/* Global function like macro defines (to be avoided): */

/****************************************************************************************/
/* Global type definitions (enum (en), struct (st), union (un), typedef (tx): */

/**
 * @brief Opaque console object
 */
typedef struct wsconsole_cmdItem_tag *wsconsole_cmdItem_tp;

/**
 * @brief Console command callback function
 * @param cmd_pt            pointer to the command structure
 * @param respStream_fp     response stream pointer
 * @return console command return code, 0 indicates "success"
 */
//typedef wserr_t (*wsconsole_callBackFunc_t)(int argc, char** argv, FILE *respStream_fp);
typedef wserr_t (*wsconsole_callBackFunc_t)(wsconsole_cmdItem_tp cmd_pt, FILE *respStream_fp);

/**
 * @brief Console get character function
 * @return one received character
 */
typedef char (*wsconsole_getCharacter_t)(void);

/**
 * @brief Console put character function
 * @param *data_pv      pointer to data channel
 * @param ch_c          character to print to channel
 * @param isLast_bol    is this the last character, than flush the complete message
 * @return none
 */
typedef void (*wsconsole_putCharacter_t)(void *data_pv, char ch_c, bool isLast_bol);

/**
 * @brief Console interrupt handler
 * @param dummy_i     dummy parameter to meet structure
 * @return none
 */
typedef void (*wsconsole_intHandler_t)(int dummy_i);

/**
 * @brief Parameters for console initialization
 */
typedef struct wsconsole_config_tag
{
    /**
     * Function pointer interface to get a character by polling.
     */
    wsconsole_getCharacter_t getCharFunc_fp;
    /**
     * Function pointer interface to print a character to the outstream
     */
    wsconsole_putCharacter_t putCharFunc_fp;
    /**
     * Function pointer interface to the interrupt handler for special 
     * characters.
     */
    wsconsole_intHandler_t intHandler_fp;
} wsconsole_config_t;

/**
 * @brief Console command description
 */
typedef struct wsconsole_cmdItem_tag{
    /**
     * Command name. Must not be NULL, must not contain spaces.
     * The pointer must be valid until the call to esp_console_deinit.
     */
    const char *command;    //!< command name
    /**
     * Help text for the command, shown by help command.
     * If set, the pointer must be valid until the call to esp_console_deinit.
     * If not set, the command will not be listed in 'help' output.
     */
    const char *help;
    /**
     * Hint text, usually lists possible arguments.
     */
    const char *hint;
    /**
     * Pointer to a callback function to handle the command.
     */
    wsconsole_callBackFunc_t func;
    /**
     * Array or structure of pointers to arg_xxx structures, may be NULL.
     * Used to generate hint text if 'hint' is set to NULL.
     * Array/structure which this field points to must end with an arg_end.
     * Only used for the duration of esp_console_cmd_register call.
     */
    void *argtable;
} wsconsole_cmdItem_t;

/**
 * @brief Abstract datatype console object
 */
typedef struct wsconsole_tag *wsconsole_tp;

/****************************************************************************************/
/* Global function definitions: */
/**---------------------------------------------------------------------------------------
 * @brief   initialize console module paramenter
 * @author  S. Wink
 * @date    05. Mar. 2024
 * @param   config_stp    pointer to configuration structure
 * @return
 *          - OK on success
 *          - ERR_PARAM if parameter set is invalid
*//*------------------------------------------------------------------------------------*/
extern wserr_t wsconsole_InitParameter_t(wsconsole_config_t *config_stp);

/**---------------------------------------------------------------------------------------
 * @brief   allocates the console
 * @author  S. Wink
 * @date    05. Mar. 2024
 * @return
 *          - allocated wsconsole opaque pointer
*//*------------------------------------------------------------------------------------*/
wsconsole_tp wsconsole_AllocateConsole_t(void);

/**---------------------------------------------------------------------------------------
 * @brief   initialize console module, Call this once before using other console module
 *              features
 * @author  S. Wink
 * @date    05. Mar. 2024
 * @param   config_stp      pointer to configuration structure
 * @param   console_x       console object
 * @return
 *          - OK on success
 *          - ERR_NO_MEM if out of memory
 *          - ERR_INVALID_STATE if already initialized
*//*------------------------------------------------------------------------------------*/
extern wserr_t wsconsole_Init_t(wsconsole_tp console_x, wsconsole_config_t *config_stp);

/**---------------------------------------------------------------------------------------
 * @brief   checks that the command is consistant and has no bad arguments/parameters.
 * @author  S. Wink
 * @date    05. Mar. 2024
 * @param   command pointer to the command definition
 * @return
 *          - wserr_OK on success
 *          - wserr_ERR_PARAM if command is not valid
*//*------------------------------------------------------------------------------------*/
extern wserr_t wsconsole_ValidateCommand_t(wsconsole_cmdItem_t *newItem_stp);
/**---------------------------------------------------------------------------------------
 * @brief   Register console command
 * @author  S. Wink
 * @date    05. Mar. 2024
 * @param   command pointer to the command definition
 * @param   console_x     console object
 * @return
 *          - OK on success
 *          - ERR_NO_MEM if out of memory
 *          - ERR_PARAM if parameter not correct
 *          - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
*//*------------------------------------------------------------------------------------*/
extern wserr_t wsconsole_RegisterCommand_t(wsconsole_tp console_x, 
                                            wsconsole_cmdItem_t *newItem_stp);

/**---------------------------------------------------------------------------------------
 * @brief   Run command line
 * @author  S. Wink
 * @date    05. Mar. 2024
 * @param   cconsole_x     console object
 * @return
 *      - ESP_OK, if command was run
 *      - ESP_ERR_INVALID_ARG, if the command line is empty, or only contained
 *        whitespace
 *      - ESP_ERR_NOT_FOUND, if command with given name wasn't registered
 *      - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
*//*------------------------------------------------------------------------------------*/
extern wserr_t wsconsole_Run_t(wsconsole_tp console_x);

/**---------------------------------------------------------------------------------------
 * @brief   de-initialize console module Call this once when done using console module
 *              functions
 * @author  S. Wink
 * @date    05. Mar. 2024
 * @param   console_x     console object
 * @return
 *          - OK on success
 *          - ERR_INVALID_STATE if not initialized yet
*//*------------------------------------------------------------------------------------*/
extern wserr_t wsconsole_DeInit_t(wsconsole_tp console_x);

/****************************************************************************************/
/* Global data definitions: */

#ifdef __cplusplus
}
#endif

#endif //MYCONSOLE_H