#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

int echo_handler(char* cmd) {
    char* cmd_string = (5+cmd);
    if(strlen(cmd) == 5) {
        printf("\n");
    }
    else {
        printf("%s", cmd_string);
    }
    fflush(stdout);
    return 0;
}

int cd_handler(char* cmd) {
    if(strlen(cmd) == 3) {
       chdir(getenv("HOME"));
    }
    else {
        char* cmd_string = (3+cmd);
        cmd_string[strlen(cmd_string)-1] = '\0';
        if(strchr(cmd_string, ' ') != NULL) {
            printf("wshell: cd: too many arguments\n");
        }
        else if(chdir(cmd_string)) { // Check if there is more than one arguement. 
            printf("wshell: no such directory: %s\n", cmd_string);
        }
    }    
    return 0;
}

int pwd_handler(char* cmd, char* cwd) {
    printf("%s\n", cwd);
    fflush(stdout);
    return 0;
}

int history_handler(char* cmd, char* cmd_history[]) {
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

int check_builtin_cmd(char* cmd, char* cwd, char* cmd_history[]) {
    int retval = 0;
    if(!strncmp(cmd, "exit", 4)) {
        exit(0);
    } 
    else if (!strncmp(cmd, "echo", 4)) {
        echo_handler(cmd);
        retval = 1;
    }
    else if (!strncmp(cmd, "cd", 2)) {
        cd_handler(cmd);
        retval = 1;
    }
    else if (!strncmp(cmd, "pwd", 3)) {
        pwd_handler(cmd, cwd);
        retval = 1;
    }
    else if (!strncmp(cmd, "history", 7)) {
        history_handler(cmd, cmd_history);
        retval = 1;
    }
    return retval;
}



int main(int argc, char* argv[]) {

    char* cmd_history[10];
    char* current_directory = malloc(100*sizeof(char));
    char* cmd = malloc(100*sizeof(char));
    char* cmd_string = malloc(100*sizeof(char));
    char* cwd, *end_cwd,  *token;
    char* cmd_argv[100];
    int retval, arg_count, PID;
    int hist_count;

    for(int i = 0; i < 10; i++) {
        cmd_history[i] = malloc(100*sizeof(char));;
    }
    hist_count = 0;

    while(1) {
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

        retval = check_builtin_cmd(cmd, cwd, cmd_history);
        if(!retval) {
            PID = fork();
            if(PID != 0) {
                waitpid(PID, &retval,0);
            }
            else {
                arg_count = 1;
                // custom command, 
                cmd[strlen(cmd)-1] = '\0'; // add nulterm to end of string
                token = strtok(cmd, " ");
                cmd_string = strcpy(cmd_string, token);
                cmd_argv[0] = cmd_string;
                while(token != NULL) {
                    token = strtok(NULL, " ");
                    cmd_argv[arg_count++] = token;
                }
                if(execvp(cmd_string, cmd_argv) == -1)  {// mybe execve?
                    printf("wshell: could not execute command: %s\n", cmd_string);
                    exit(1);
                }
            }
        }   
    }
    return 0;
}
