#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void shell();

char history[10][128];
int count = 0;
int backJobIds[255] = {0};    
pid_t backPids[255] = {0};      
char backCommands[255][128];    
bool backRunning[255] = {false};
int nextJobId = 0;
int activeJobs = 0;

int main(int argc, char* argv[]) {
    shell();
    return 0;
}

int builtIn (char* newArr[], char* path, char* shortenedPath, char* fullArguments){
    int i = 0;
    while (newArr[i] != NULL){
        i++;
    }
    char checkls[128];
    strcpy(checkls, newArr[0]);
    char *last_slash = strrchr(checkls, '/');
    if (last_slash != NULL) {
        strcpy(checkls, last_slash + 1);
    }
    if (strcmp(checkls, "cd") == 0) {
        if (i > 2) {
            printf("wshell: cd: too many arguments\n");
            return 0;
        }
        if (i == 1) {  
            char *home = getenv("HOME");
            if (home != NULL && chdir(home) == 0) {
                getcwd(path, 100);
                char *last_slash = strrchr(path, '/');
                if (last_slash != NULL) {
                    if (*(last_slash + 1) == '\0' || last_slash == path) {
                        strcpy(shortenedPath, "/");
                    } else {
                        strcpy(shortenedPath, last_slash + 1);
                    }
                }
                return 0;
            }
            return 0;
        }
        if (chdir(newArr[1]) != 0) {
            printf("wshell: no such directory: %s\n", newArr[1]);
            return 0;
        } else {
            getcwd(path, 100);
            char *last_slash = strrchr(path, '/');
            if (last_slash != NULL) {
                if (*(last_slash + 1) == '\0') {
                    strcpy(shortenedPath, "/");
                } else {
                    if (strcmp(shortenedPath, last_slash+1) == 0) {
                        strcpy(shortenedPath, newArr[1]);
                    } else {
                        strcpy(shortenedPath, last_slash + 1);
                    }
                }
            }
            return 0;
        }
    } else if (strcmp(checkls, "jobs") == 0){
        for (int j = 0; j < 255; j++) {
            if (backRunning[j]) {
                // making sure its terminated properly
                backCommands[j][sizeof(backCommands[j]) - 1] = '\0';
                printf("%d: %s \n", backJobIds[j], backCommands[j]);
            }
        }
        return 0;
    }else if (strcmp(checkls, "kill") == 0){
        int jobId = atoi(newArr[1]);
        bool found = false;
        for (int j = 0; j < 255; j++) {
            if (backRunning[j] && backJobIds[j] == jobId) {
                kill(backPids[j], SIGTERM);
                found = true;
                backRunning[j] = false;
                break;
            }
        }
        if (!found) {
            printf("wshell: no such background job: %d\n", jobId);
        }
        return 0;
    }else if (strcmp(checkls, "pwd") == 0) {
        printf("%s\n", path);
        return 0;
    } else if (strcmp(checkls, "echo") == 0) {
        for (int j = 1; j < i - 1; j++) {
            printf("%s ", newArr[j]);
        }
        if (i > 1) {
            printf("%s", newArr[i-1]);
        }
        printf("\n");
        return 0;
    } else if (strcmp(checkls, "history") == 0) {
        int start;
        if (count > 10){
            start = count - 10;
        }
        else {
            start = 0;
        }
        for (int i = start; i < count; i++) {
            printf("%s\n", history[i%10]);
        }
        return 0;
    }
    return 1;
}

void checkBackgroundJobs() {
    int status;
    pid_t pid;
   
    // wait for any child process
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < 255; i++) {
            if (backRunning[i] && backPids[i] == pid) {
                printf("[%d] Done: %s \n", backJobIds[i], backCommands[i]);
                backRunning[i] = false;
                activeJobs--;
                break;
            }
        }
    }
}

int findAvailableJobId() {
    nextJobId++;
    if (nextJobId > 255) nextJobId = 1;
    return nextJobId;
}

void addBackgroundJob(pid_t pid, char* cmd) {
    int jobId = findAvailableJobId();
    for (int i = 0; i < 255; i++) {
        if (!backRunning[i]) {
            backJobIds[i] = jobId;
            backPids[i] = pid;
            // trying to avoid buffer and memory issues but failing
            memset(backCommands[i], 0, sizeof(backCommands[i]));
            strncpy(backCommands[i], cmd, sizeof(backCommands[i]) - 1);
            backRunning[i] = true;
            activeJobs++;
            printf("[%d]\n", jobId);
            break;
        }
    }
}

int command(char* newArr[], bool background) {
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
    } else if (pid == 0) {
        execvp(newArr[0], newArr);
        printf("wshell: could not execute command: %s\n", newArr[0]);
        exit(1);
    } else {  
        if (background){
            char cmd[128];
            // going super overboard because there were so many memory buffer issues
            memset(cmd, 0, sizeof(cmd));
            // Build command string safely
            size_t remaining = sizeof(cmd);
            size_t offset = 0;
            int i = 0;
            while (newArr[i] != NULL && remaining > 1) {
                if (i > 0 && remaining > 1) {
                    strncat(cmd + offset, " ", remaining - 1);
                    offset = strlen(cmd);
                    remaining = sizeof(cmd) - offset;
                }
                strncat(cmd + offset, newArr[i], remaining - 1);
                offset = strlen(cmd);
                remaining = sizeof(cmd) - offset;
                i++;
            }
            addBackgroundJob(pid, cmd);
            return 0;
        }else {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            return 1;
        }
    }
    return 0;
}

void handleAndOrCommands(char* firstCmd[], char* secondCmd[], bool isAnd, char* path, char* shortenedPath, char* fullArguments) {
    int builtin = builtIn(firstCmd, path, shortenedPath, fullArguments);
    bool shouldRunSecond;
    if (builtin != 0){
        int firstStatus = command(firstCmd, false);
        shouldRunSecond = (isAnd && firstStatus == 0) || (!isAnd && firstStatus != 0);
    }
    else {
        shouldRunSecond = (isAnd && builtin == 0) || (!isAnd && builtin != 0);
    }

    if (shouldRunSecond){
        int builtin1 = builtIn(secondCmd, path, shortenedPath, fullArguments);
        if (builtin1 != 0){
            command(secondCmd, false);
        }
    }
}

int executeWithRedirection(char* cmd[], char* file, bool shouldAppend) {
    pid_t pid = fork();
   
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }
   
    if (pid == 0) {
        int fd;
        if (shouldAppend) {
            fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
            fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
       
        if (fd < 0) {
            perror("open failed");
            exit(1);
        }

        // test 2
        fclose(stdout);
       
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 failed");
            exit(1);
            close(fd);
        }
        close(fd);
       
        execvp(cmd[0], cmd);
        perror("execvp failed");
        exit(1);
    } else {
        // parent wait on child
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}


int executePipe(char* cmd1[], char* cmd2[]) {
    int pipefd[2];
    pid_t pid1, pid2;
    if (pipe(pipefd) < 0) {
        perror("pipe failed");
        return 1;
    }
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork failed");
        return 1;
    }else if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd1[0], cmd1);
        perror("execvp failed");
        exit(1);
    }
   
    //2nd child reading
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        return 1;
    }
   
    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(cmd2[0], cmd2);
        perror("execvp failed");
        exit(1);
    }
   
    // remembering to close both ends of the pipe since were not using it
    close(pipefd[0]);
    close(pipefd[1]);

    // wait for both of the children
    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);
   
    return WEXITSTATUS(status2);
}

void addHistory(char* command){
    if (count < 10) {
        strcpy(history[count], command);
        count++;
    } else {
        for (int i = 1; i < 10; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[10 - 1], command);
    }
}

void shell() {
    char prompt[128] = "$ ";
    char arguments[128];
    char fullArguments[128];
    char path[100];
    char shortenedPath[50];

    // get current home directory
    getcwd(path, sizeof(path));
    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        if (*(last_slash + 1) == '\0' || last_slash == path) {
            strcpy(shortenedPath, "/");
        } else {
            strcpy(shortenedPath, last_slash + 1);
        }
    }

    while (true) {
        checkBackgroundJobs();
        // prints prompt based off of highest directory
        printf("%s%s", shortenedPath, prompt);
        if (fgets(arguments, sizeof(arguments), stdin) == NULL) {
            exit(1);
        }
        if (!isatty(fileno(stdin))) {
            printf("%s", arguments);
            fflush(stdout);
        }
        arguments[strcspn(arguments, "\n")] = '\0';
        strcpy(fullArguments, arguments);
           
        // split up the arguments by space and put them in a new array
        char* newArr[64] = {0};
        int i = 0;
        char* token = strtok(arguments, " ");
        while (token != NULL && i < 63) {
            newArr[i++] = token;
            token = strtok(NULL, " ");
        }
        newArr[i] = NULL;

        bool background = false;
        if (i > 0 && strcmp(newArr[i-1], "&") == 0) {
            background = true;
            newArr[i-1] = NULL;
            i--;
        }

        if (i == 0) continue;

        // if they just want to exit we don't want to go through all of this code  
        if (strcmp(newArr[0], "exit") == 0) {
            exit(0);
        }
        int index = -1;
        int pipeIndex = -1;
        int redirectIndex = -1;
        bool isAnd = false;
        bool shouldAppend = false;

        // check if && or || are in the command, if they are, it breaks it up into 2 different commands
        for (int j = 1; j < i; j++) {
            if (strcmp(newArr[j], "&&") == 0) {
                isAnd = true;
                index = j;
                break;
            } else if (strcmp(newArr[j], "||") == 0) {
                index = j;
                break;
            }
        }

        if (index == -1) {
            for (int j = 1; j < i; j++) {
                if (strcmp(newArr[j], ">") == 0) {
                    redirectIndex = j;
                    newArr[j] = NULL;
                    break;
                }
                else if (strcmp(newArr[j], ">>") == 0) {
                    shouldAppend = true;
                    redirectIndex = j;
                    break;
                }
            }
        }

        if (index == -1 && redirectIndex == -1){
            for (int j = 1; j < i; j++) {
                if (strcmp(newArr[j], "|") == 0) {
                    pipeIndex = j;
                    break;
                }
            }
        }

        if (index != -1) {
            newArr[index] = NULL;
            char** firstCmd = newArr;
            char** secondCmd = &newArr[index + 1];
            handleAndOrCommands(firstCmd, secondCmd, isAnd, path, shortenedPath, fullArguments);
            addHistory(fullArguments);
            continue;
        } else if (pipeIndex != -1){
            newArr[pipeIndex] = NULL;
            char** cmd1 = newArr;
            char** cmd2 = &newArr[pipeIndex + 1];
            executePipe(cmd1, cmd2);
        } else if (redirectIndex != -1){
            char* file = newArr[redirectIndex + 1];
            newArr[redirectIndex] = NULL;
            executeWithRedirection(newArr, file, shouldAppend);
        }else {
            addHistory(fullArguments);
            if (background) {
                // background process could possibly be built in
                pid_t pid = fork();
                if (pid == 0) {
                    // Child
                    int builtin = builtIn(newArr, path, shortenedPath, fullArguments);
                    if (builtin != 0) {
                        fflush(stdout);
                        execvp(newArr[0], newArr);
                        printf("wshell: could not execute command: %s\n", newArr[0]);
                        exit(1);
                    }
                    exit(0);
                } else {
                    // Parent
                    char cmd[128];
                    memset(cmd, 0, sizeof(cmd));
                    size_t remaining = sizeof(cmd);
                    size_t offset = 0;
                    int i = 0;
                    // being extremely careful about memory and buffers
                    while (newArr[i] != NULL && remaining > 1) {
                        if (i > 0 && remaining > 1) {
                            strncat(cmd + offset, " ", remaining - 1);
                            offset = strlen(cmd);
                            remaining = sizeof(cmd) - offset;
                        }
                        strncat(cmd + offset, newArr[i], remaining - 1);
                        offset = strlen(cmd);
                        remaining = sizeof(cmd) - offset;
                        i++;
                    }
                    addBackgroundJob(pid, cmd);
                }
            } else {
                // handle as we have been
                int builtin = builtIn(newArr, path, shortenedPath, fullArguments);
                if (builtin != 0) {
                    command(newArr, false);
                }
            }
        }
    }
}