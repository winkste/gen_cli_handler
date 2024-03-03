/****************************************************************************************
* FILENAME :        main.c
*
* SHORT DESCRIPTION:
*   CLI basics using argtable3 and embedded_cli libraries. 
*
* DETAILED DESCRIPTION :     
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
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <termios.h>
#include <unistd.h>

#include "embedded_cli.h"
#include "argtable3.h"

/***************************************************************************************/
/* Local constant defines */
#define MAX_ARGS 10 // Maximum number of arguments

/***************************************************************************************/
/* Local function like makros */

/***************************************************************************************/
/* Local type definitions (enum, struct, union) */

/***************************************************************************************/
/* Local functions prototypes: */
static char getch(void);
static void intHandler(int dummy);
static void posix_putch(void *data, char ch, bool is_last);

/***************************************************************************************/
/* Local variables: */
static struct embedded_cli cli;

/***************************************************************************************/
/* Global functions (unlimited visibility) */
/**--------------------------------------------------------------------------------------
 * @brief     Main function and entry point for executable
 * @author    S. Wink
 * @date      03. Mar. 2024
*//*-----------------------------------------------------------------------------------*/
int main(void)
{
    bool done = false;
    char progname[] = "arg3cli.exe";

    struct arg_lit *help_cmd = arg_litn(NULL, "help", 0, 1, "display help for the program");
    struct arg_lit *add_cmd  = arg_lit0(NULL, "add", "Add two numbers");
    struct arg_int *add_args[2];
    add_args[0] = arg_int0(NULL, NULL, "<a>", "First number");
    add_args[1] = arg_int0(NULL, NULL, "<b>", "Second number");
    struct arg_lit *exit_cmd = arg_litn(NULL, "exit", 0, 1, "exit the program");
    struct arg_end *end = arg_end(20);

    void *argtable[] = {help_cmd, exit_cmd, add_cmd, add_args[0], add_args[1], end};

    arg_print_glossary(stdout, argtable, "  %-25s %s\n");

    /**
     * Start up the Embedded CLI instance with the appropriate
     * callbacks/userdata
     */
    embedded_cli_init(&cli, "cli> ", posix_putch, stdout);
    embedded_cli_prompt(&cli);

    /* Capture Ctrl-C */
    signal(SIGINT, intHandler);

    while (!done) {
        char ch = getch();

        /**
         * If we have entered a command, try and process it
         */
        if (embedded_cli_insert_char(&cli, ch)) {
            int cli_argc;
            char **cli_argv;
            cli_argc = embedded_cli_argc(&cli, &cli_argv);

            int argc = 1; // Start with 1 to account for program name
            char *argv[MAX_ARGS];  // Array to store arguments
            argv[0] = progname; // Program name as first argument
            printf("Got %d args\n", cli_argc);
            for (int i = 0; i < cli_argc; i++) {
                printf("Arg %d/%d: '%s'\n", i, cli_argc, cli_argv[i]);
                argv[i + 1] = cli_argv[i];
                argc++;
            }
            // Print the copied argc and argv
            printf("Copied argc: %d\n", argc);
            for (int i = 0; i < argc; i++)
            {
                printf("Copied Arg %d/%d: '%s'\n", i, argc, argv[i]);
            }

            // Parse the arguments
            int nerrors = arg_parse(argc, argv, argtable);

            if (help_cmd->count > 0) /* '--help' */
            {
                printf("Usage: %s", progname);
                arg_print_syntax(stdout, argtable, "\n");
                printf("Demonstrate command-line parsing in argtable3.\n\n");
                arg_print_glossary(stdout, argtable, "  %-25s %s\n");
            }
            else if(add_cmd->count > 0) /* '--add' */
            {
                if(nerrors == 0 && add_args[0]->count > 0 && add_args[1]->count > 0) 
                {
                    int result = *add_args[0]->ival + *add_args[1]->ival;
                    printf("Result of addition: %d\n", result);
                } 
                else 
                {
                    printf("Usage: --add <a> <b>\n");
                }
            } 
            else if (exit_cmd->count > 0) /* '--help' */
            {
                printf("Exiting...\n");
                break;
            }
            else 
            {
                printf("No command specified.\n");
            }
            
            //done = cli_argc >= 1 && (strcmp(cli_argv[0], "quit") == 0);

            //if (!done)
            embedded_cli_prompt(&cli);
        }
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
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
static char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");

    struct termios raw = old;

    // Do what cfmakeraw does (Using --std=c99 means that cfmakeraw isn't
    // available)
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

    if (read(0, &buf, 1) < 0)
        perror("read()");

    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

/**--------------------------------------------------------------------------------------
 * @brief     The intHandler function is a signal handler for SIGINT (Ctrl-C). It's used
 *            to capture Ctrl-C and interrupt the CLI input.
 * @author    S. Wink
 * @date      03. Mar. 2024
 * @param     dummy             input data pointer
 * @return    none
*//*-----------------------------------------------------------------------------------*/
static void intHandler(int dummy)
{
    (void)dummy;
    embedded_cli_insert_char(&cli, '\x03');
}

/**--------------------------------------------------------------------------------------
 * @brief     This function outputs a single character to stdout, to be used as the
 *            callback from embedded cli.
 * @author    S. Wink
 * @date      03. Mar. 2024
 * @param     *data     pointer to data channel
 * @param     ch        character to print to channel
 * @param     is_last   is this the last character, than flush the complete message
 * @return    none
*//*-----------------------------------------------------------------------------------*/
static void posix_putch(void *data, char ch, bool is_last)
{
    FILE *fp = data;
    fputc(ch, fp);
    if (is_last)
        fflush(fp);
}