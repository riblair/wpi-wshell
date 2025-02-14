
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int BUFFER_MAX = 512;

void print_file(char* filename, char* search_term) {

    FILE* fp = fopen(filename, "r");
    size_t* buf_size = malloc(sizeof(int));
    char* buffer = malloc(sizeof(char)*BUFFER_MAX);

    if(!fp) {
        printf("wgrep: cannot open file\n");
        exit(1);
    }

    while(getline(&buffer, buf_size, fp) != -1) {
        if (strstr(buffer, search_term) != NULL)
            fputs(buffer, stdout);
    }
}

void print_std_in(char* search_term) {
    size_t* buf_size = malloc(sizeof(int));
    char* buffer = malloc(sizeof(char)*BUFFER_MAX);
    while(getline(&buffer, buf_size, stdin) != -1) {
        if (strstr(buffer, search_term) != NULL)
            fputs(buffer, stdout);
    }
}

int main(int argc, char* argv[]) {

    switch(argc) {
        case 1: // error, not enough args passed
            printf("wgrep: searchterm [file ...]\n");
            exit(1);
        break;

        case 2: // search term passed, should accept input from stdin
            // make a fnc for this
            print_std_in(argv[1]);
        break;

        default: // 1 or more files passed into the args, parse each file  
            for(int i = 2; i < argc; i++) {
                print_file(argv[i], argv[1]);
            }
        break;
    }
    exit(0);
}