#include <stdio.h>
#include <string.h>
#include "argtable3.h"

#define MAX_ARGS 10 // Maximum number of arguments

int main(void)
{
    char input[500];
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

    while (1) 
    {
        printf("Enter a command: ");
        fgets(input, sizeof(input), stdin);

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // Tokenize input string into separate arguments
        int argc = 1; // Start with 1 to account for program name
        char *argv[MAX_ARGS];  // Array to store arguments
        argv[0] = progname; // Program name as first argument
        char *token = strtok(input, " ");
        while (token != NULL && argc < MAX_ARGS) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
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
    }   

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}
