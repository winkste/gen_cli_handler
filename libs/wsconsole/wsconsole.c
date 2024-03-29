/****************************************************************************************
* FILENAME :        wsconsole.c
*
* SHORT DESCRIPTION:
*   Implementation for console parsing independent on the input channel
*
* DETAILED DESCRIPTION :     
*
* AUTHOR :    Stephan Wink        CREATED ON :    04. MAr 2024
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
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
****************************************************************************************/

/***************************************************************************************/
/* Include Interfaces */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "wsconsole.h"

#include "wserr.h"
#include "embedded_cli.h"
#include "argtable3.h"
#include "queue.h"

/***************************************************************************************/
/* Local constant defines */

/***************************************************************************************/
/* Local function like makros */

/***************************************************************************************/
/* Local type definitions (enum, struct, union) */

/**
 * @brief Console state machine handling
 */
typedef enum consoleState_tag
{
    STATE_ALLOCATED = 0,
    STATE_INITIALIZED,

}consoleState_t;

/**
 * @brief Command list single item
 */
typedef struct cmdItem_tag
{
    /**
     * this command item
     */
    wsconsole_cmdItem_t thisItem_st;
    /**
     * next list item
     */
    SLIST_ENTRY(cmdItem_tag) nextItem_st;  //!< next command in the list
}cmdItem_t;

/**
 * @brief Console object implementation
 */
typedef struct wsconsole_tag
{
    /**
     * Configuration of the console
     */
    wsconsole_config_t config_st;
    /**
     * Linked list of all registered commands
     */
    SLIST_HEAD(cmd_list_, cmdItem_tag) cmdList_st;
    /**
     * Command line interface object
     */
    struct embedded_cli cli_st;
    /**
     * console state
    */
    consoleState_t state_en;
}wsconsole_t;

/***************************************************************************************/
/* Local functions prototypes: */
static cmdItem_t *FindCommandByName_stp(wsconsole_tp console_x, const char *name_cpc);
static int HelpCommand_i(wsconsole_cmdItem_tp cmd_pt, FILE *respStream_fp);
static wserr_t RegisterHelpCommand_t(wsconsole_tp console_x);

/***************************************************************************************/
/* Local variables: */
/**
 * Singleton object implementation
 */
static wsconsole_t console_sx;
/***************************************************************************************/
/* Global functions (unlimited visibility) */

/**--------------------------------------------------------------------------------------
 * @brief     Initializes the initialization structure of the basic module
 * @author    S. Wink
 * @date      07 Mar. 2024
*//*-----------------------------------------------------------------------------------*/
wserr_t wsconsole_InitParameter_t(wsconsole_config_t *config_stp)
{
    wserr_t exeResult_st = wserr_ERR_GEN;

    if(config_stp != NULL)
    {
        exeResult_st = wserr_OK;  
        config_stp->getCharFunc_fp = NULL;
        config_stp->intHandler_fp = NULL;
        config_stp->putCharFunc_fp = NULL; 
    }

    return(exeResult_st);
}

/**--------------------------------------------------------------------------------------
 * @brief     Allocate the console
 * @author    S. Wink
 * @date      07 Mar. 2024
*//*-----------------------------------------------------------------------------------*/
wsconsole_tp wsconsole_AllocateConsole_t(void)
{
    /* new allocation, reset status */
    console_sx.state_en = STATE_ALLOCATED;
    /* Currently only support singleton in this version */
    return(&console_sx);
}

/**--------------------------------------------------------------------------------------
 * @brief     Initialization of the console
 * @author    S. Wink
 * @date      07 Mar. 2024
*//*-----------------------------------------------------------------------------------*/
wserr_t wsconsole_Init_t(wsconsole_tp console_x, wsconsole_config_t *config_stp)
{
    wserr_t exeResult_st = wserr_OK;

    /* Validation of parameter list first */
    if((config_stp == NULL) || (console_x == NULL))
        return wserr_ERR_PARAM;

    /* Do the initialization only in when in state allocated */
    if(console_x->state_en != STATE_ALLOCATED)
        return wserr_ERR_INVALID_STATE;
    

    /* Copy parameter for execution */
    memcpy(&console_x->config_st, config_stp, sizeof(wsconsole_config_t));

    /* Start up the Embedded CLI instance with the appropriate
     * callbacks/userdata */
    embedded_cli_init(&console_x->cli_st, "cli> ", 
                        console_x->config_st.putCharFunc_fp, stdout);
    
    /* Capture Ctrl-C in an interrupt service routine */
    struct sigaction sa;
    sa.sa_handler = console_x->config_st.intHandler_fp;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    /* Register signal handler for SIGINT (Ctrl-C) using sigaction */
    if (sigaction(SIGINT, &sa, NULL) == -1) 
    {
        exeResult_st = wserr_ERR_GEN;   
    }

    /* Initialization without errors, move the initialized state */
    if(wserr_OK == exeResult_st)
    {
        console_x->state_en = STATE_INITIALIZED;
    }

    /* Register as basic the help function */
    if(wserr_OK == exeResult_st)
    {
        exeResult_st = RegisterHelpCommand_t(console_x);
    }
    
    return(exeResult_st);
}

/**--------------------------------------------------------------------------------------
 * @brief     Validate command
 * @author    S. Wink
 * @date      07 Mar. 2024
*//*-----------------------------------------------------------------------------------*/
wserr_t wsconsole_ValidateCommand_t(wsconsole_cmdItem_t *newItem_stp)
{
    wserr_t exeResult_st = wserr_OK;

    if((newItem_stp != NULL) 
        && (newItem_stp->command != NULL) 
        && (newItem_stp->func != NULL))
    {
        exeResult_st = wserr_OK;
    }
    else
    {
        exeResult_st = wserr_ERR_PARAM;
    }

    return(exeResult_st);
}

/**--------------------------------------------------------------------------------------
 * @brief     Register new command
 * @author    S. Wink
 * @date      07 Mar. 2024
*//*-----------------------------------------------------------------------------------*/
wserr_t wsconsole_RegisterCommand_t(wsconsole_tp console_x, 
                                            wsconsole_cmdItem_t *newItem_stp)
{
    wserr_t exeResult_st = wserr_ERR_GEN;
    cmdItem_t *tempItem_stp;
    cmdItem_t *lastItem_stp;

    /* Check the internal structure of the command */
    exeResult_st = wsconsole_ValidateCommand_t(newItem_stp);
    if(wserr_OK == exeResult_st)
    {
        /* Allocate memory for new command */
        tempItem_stp = (cmdItem_t *) calloc(1, sizeof(wsconsole_cmdItem_t));
        if(NULL != tempItem_stp)
        {
            /* copy the data only to the allocated temporary object */
            memcpy(&tempItem_stp->thisItem_st, newItem_stp, sizeof(wsconsole_cmdItem_t));
                lastItem_stp = SLIST_FIRST(&console_x->cmdList_st);
            /* add command item to command list */
            if (NULL == lastItem_stp) 
            {
                SLIST_INSERT_HEAD(&console_x->cmdList_st, tempItem_stp, nextItem_st);
            } else 
            {
                cmdItem_t *it;
                while ((it = SLIST_NEXT(lastItem_stp, nextItem_st)) != NULL) {
                    lastItem_stp = it;
                }
                SLIST_INSERT_AFTER(lastItem_stp, tempItem_stp, nextItem_st);
            }
        }
        else
        {
            /* allocation of new item failed */
            exeResult_st = wserr_ERR_NO_MEM;
        }
    }

    return(exeResult_st); 
}

/**--------------------------------------------------------------------------------------
 * @brief     Run function
 * @author    S. Wink
 * @date      07 Mar. 2024
*//*-----------------------------------------------------------------------------------*/
wserr_t wsconsole_Run_t(wsconsole_tp console_x)
{
    wserr_t exeResult_st = wserr_ERR_GEN;
    int nerrors = 0;
    bool done = false;
    cmdItem_t *cmd_pt = NULL;
    char response_c[500];
    int cli_argc;
    char **cli_argv;

    embedded_cli_prompt(&console_x->cli_st);
    while (!done) {
        char ch = console_x->config_st.getCharFunc_fp();

        /**
         * If we have entered a command, try and process it
         */
        if (embedded_cli_insert_char(&console_x->cli_st, ch)) {
            cli_argc = embedded_cli_argc(&console_x->cli_st, &cli_argv);

            cmd_pt = FindCommandByName_stp(console_x, cli_argv[0]);
            if(NULL != cmd_pt)
            {
                if(cmd_pt->thisItem_st.argtable != NULL)
                {
                    nerrors = arg_parse(cli_argc, cli_argv, cmd_pt->thisItem_st.argtable);
                }
                if(0 == nerrors)
                {
                    FILE* memstream = fmemopen(response_c, 500, "w");
                    exeResult_st = cmd_pt->thisItem_st.func((wsconsole_cmdItem_t *)cmd_pt,
                                                                memstream);
                    embedded_cli_response(&console_x->cli_st, response_c);
                    fclose(memstream);
                }
            }
            embedded_cli_prompt(&console_x->cli_st);
        }
    }
    return(exeResult_st);    
}

/**--------------------------------------------------------------------------------------
 * @brief     Stops the basic module
 * @author    S. Wink
 * @date      13. Jan. 2020
*//*-----------------------------------------------------------------------------------*/
wserr_t wsconsole_DeInit_t(wsconsole_tp console_x)
{
    wserr_t exeResult_st = wserr_ERR_GEN;
    cmdItem_t *it, *tmp;

    if(console_x != NULL)
    {
        SLIST_FOREACH_SAFE(it, &console_x->cmdList_st, nextItem_st, tmp)
        free(it);
        console_x->state_en = STATE_ALLOCATED;
        exeResult_st = wserr_OK;
    }

    return(exeResult_st);    
}

/***************************************************************************************/
/* Local functions: */

/**---------------------------------------------------------------------------------------
 * @brief   Search in the command list and find the command by key "name"
 * @author  S. Wink
 * @date    09. Mar. 2024
 * @param[in]   name_cpc command name
 * @return      pointer to the command item, else NULL
*//*------------------------------------------------------------------------------------*/
static cmdItem_t *FindCommandByName_stp(wsconsole_tp console_x, const char *name_cpc)
{
    cmdItem_t *cmd_stp = NULL;
    cmdItem_t *item_stp;
    SLIST_FOREACH(item_stp, &console_x->cmdList_st, nextItem_st)
    {
        if (strcmp(name_cpc, item_stp->thisItem_st.command) == 0)
        {
            cmd_stp = item_stp;
            break;
        }
    }
    return cmd_stp;
}

/**---------------------------------------------------------------------------------------
 * @brief   Help command function, prints all commands registered to console
 * @author  S. Wink
 * @date    10. Mar. 2024
 * @param[in]   argc    number of arguments
 * @param[in]   argv    argument pointer vector
 * @param[out]  resp_pc pointer to zero terminated string response
 * @return      wserr_OK
*//*------------------------------------------------------------------------------------*/
static int HelpCommand_i(wsconsole_cmdItem_tp cmd_pt, FILE *respStream_fp)
{
    cmdItem_t *item_stp;
    wsconsole_cmdItem_t *cmd_stp;
    int lineFeed_i = 0;

    /* Print summary of each command */
    SLIST_FOREACH(item_stp, &console_sx.cmdList_st, nextItem_st)
    {
        cmd_stp = &item_stp->thisItem_st;
        if (cmd_stp->help == NULL)
        {
            continue;
        }
        /* First line: command name and hint
         * Pad all the hints to the same column
         */
        fprintf(respStream_fp, "%-s ", cmd_stp->command);
        if(cmd_stp->hint != NULL)
        {
            fprintf(respStream_fp, "%s\n", cmd_stp->hint);
        }
        else
        {
            fprintf(respStream_fp, " - NO HINT\n");    
        }
        
        /* Second line: print help.
         * Argtable has a nice helper function for this which does line
         * wrapping.
         */
        arg_print_formatted(respStream_fp, 2, 78, cmd_stp->help);
        /* Finally, print the list of arguments */
        if (cmd_stp->argtable)
        {
            arg_print_glossary(respStream_fp, 
                                (void **) cmd_stp->argtable, "  %12s  %s\n");
        }
    }
    return 0U;
}

/**---------------------------------------------------------------------------------------
 * @brief Register a 'help' command
*//*------------------------------------------------------------------------------------*/
static wserr_t RegisterHelpCommand_t(wsconsole_tp console_x)
{
    wsconsole_cmdItem_t command_st = 
    {
        .command = "help",
        .help = "Print the list of registered commands",
        .argtable = NULL,
        .func = &HelpCommand_i
    };

    return wsconsole_RegisterCommand_t(console_x, &command_st);
}



