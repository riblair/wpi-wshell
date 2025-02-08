#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>


//TODO: currently not preserving spaces during this implementation of the echo cmd. 
// Must find a better way to deal w/ this. 
// substring everything after the "echo " in cmd 
int echo_handler(char* cmd_substr) {
    printf("%s\n", (cmd_substr+5));
    fflush(stdout);
    return 0;
}

int cd_handler(char* cmd_argv[], int argv_count) {
    int retval = 1;
    if(argv_count == 1) { // only cd called
       chdir(getenv("HOME"));
       retval = 0;
    }
    else if (argv_count == 2) {
        retval = abs(chdir(cmd_argv[1]));
        if(retval) printf("wshell: no such directory: %s\n", cmd_argv[1]);
    }
    else {
        printf("wshell: cd: too many arguments\n");
    } 
    return retval;
}

int pwd_handler() {
    char* cwd = getcwd(malloc(100*sizeof(char)), 100);
    printf("%s\n", cwd);
    fflush(stdout);
    return 0;
}

int history_handler(char* cmd_history[]) {
    int hist_iter = 9;

    for(int i = 9; i >= 0; i--) {
        if(cmd_history[hist_iter] == NULL) {
            continue;
        }
        printf("%s", cmd_history[hist_iter--]);
    }
    fflush(stdout);
    return 0;
}

void update_cmd_history(char* cmd_history[], char* cmd, int* hist_count) {
    // take new entry and update old list
    for(int i = *hist_count; i > 0; i--) {
        strcpy(cmd_history[i], cmd_history[i-1]); // move pointers over by 1
    }
    strcpy(cmd_history[0], cmd);
    if (*hist_count != 9) {
        *hist_count = *hist_count + 1;
    }
}

// returns 0 on success, 1 on failure, and -1 if no cmd run.
int check_builtin_cmd(char* cmd_substr, char* cmd_argv[], int argv_count, char* cmd_history[]) {
    int retval = -1;
    if(!strncmp(cmd_argv[0], "exit", 4)) exit(0);
    else if (!strncmp(cmd_argv[0], "echo", 4)) retval = echo_handler(cmd_substr);
    else if (!strncmp(cmd_argv[0], "cd", 2)) retval = cd_handler(cmd_argv,argv_count);
    else if (!strncmp(cmd_argv[0], "pwd", 3)) retval = pwd_handler();
    else if (!strncmp(cmd_argv[0], "history", 7)) retval = history_handler(cmd_history);
    return retval;
}

int child_handler(char* cmd_argv[]) {
    int PID = fork();
    int retval = 0;

    if(PID) { //parent
        waitpid(PID, &retval, 0);
    }
    else { //child
        if(execvp(cmd_argv[0], cmd_argv) == -1)  {// mybe execve?
            printf("wshell: could not execute command: %s\n", cmd_argv[0]);
            exit(1);
        }
    }
    return retval;
}

int main(int argc, char* argv[]) {

    char* cmd_history[10];
    char* current_directory = malloc(100*sizeof(char));
    char* cmd = malloc(100*sizeof(char));
    char* cmd_ref = malloc(100*sizeof(char));
    char* cmd_1_substr = malloc(50*sizeof(char));
    char* cmd_2_substr = malloc(50*sizeof(char));
    char* cwd, *end_cwd, *token, *delim_substr;
    char* cmd_tokens[100];
    char* cmd_argv[100];
    char* cmd_2_argv[100];
    int retval, hist_count, token_iter;
    int arg_count, arg2_count, token_count;
    int and_mode, or_mode;
    int delim_position;
    for(int i = 0; i < 10; i++) {
        cmd_history[i] = malloc(100*sizeof(char));;
    }
    hist_count = 0;

    while(1) {
        delim_substr = NULL;
        delim_position = 0;
        arg_count = 0;
        arg2_count = 0;
        token_count = 0;
        token_iter = 0;
        and_mode = 0;
        or_mode = 0;


        // print the cwd
        cwd = getcwd(current_directory, 100);
        end_cwd = basename(cwd);

        printf("%s$ ", end_cwd);
        fflush(stdout);

        // get user inpuit
        fgets(cmd, 100, stdin);
        if (!isatty(fileno(stdin))) {
            printf("%s", cmd);
            fflush(stdout);
        }
        update_cmd_history(cmd_history, cmd, &hist_count);

        
        cmd[strlen(cmd)-1] = '\0';
        strcpy(cmd_ref, cmd);
        token = strtok(cmd, " ");
        cmd_tokens[token_count++] = token;
        while (token != NULL) {
            token = strtok(NULL, " ");
            cmd_tokens[token_count++] = token;
        }
        token_count--;
            
        while (token_iter < token_count) {
            if(!strcmp(cmd_tokens[token_iter], "&&")) {
                and_mode = 1;
                cmd_argv[token_iter++] = NULL;

                delim_substr = strstr(cmd_ref, " &&");
                delim_position = delim_substr - cmd_ref;
                break;
            }
            else if (!strcmp(cmd_tokens[token_iter], "||")) {
                or_mode = 1;
                cmd_argv[token_iter++] = NULL;

                delim_substr = strstr(cmd_ref, " ||");
                delim_position = delim_substr - cmd_ref;
                break;
            }
            else {
                cmd_argv[token_iter] = cmd_tokens[token_iter];
            }
            token_iter++;                
        }
        arg_count = token_iter;
        while (token_iter < token_count) {
                cmd_2_argv[arg2_count++] = cmd_tokens[token_iter++];
        }

        // seperate cmd strings 1 and 2 into substrings for echo handler
        if(!delim_position) {
            // only 1 command was entered, entire string is cmd1
            cmd_1_substr = cmd_ref;
        }
        else {
            cmd_1_substr = strncpy(cmd_1_substr, cmd_ref, delim_position);
            cmd_2_substr = (cmd_ref+delim_position+4); // delims are two characters w/ a space on either side
        }
        retval = check_builtin_cmd(cmd_1_substr, cmd_argv, arg_count, cmd_history);
        if(retval == -1) {
            retval = child_handler(cmd_argv);
        }
        if(!retval && and_mode) {
            retval = check_builtin_cmd(cmd_2_substr, cmd_2_argv, arg2_count, cmd_history);
            if (retval == -1) child_handler(cmd_2_argv);
        }
        else if (retval && or_mode) {
            retval = check_builtin_cmd(cmd_2_substr, cmd_2_argv, arg2_count, cmd_history);
            if (retval == -1) child_handler(cmd_2_argv);
        } 
    }
    return 0;
}
