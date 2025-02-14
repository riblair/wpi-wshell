#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>


void print_file(char* filename) {

    FILE* fp = fopen(filename, "r");
    int c;

    if(!fp) {
        printf("wcat: cannot open file\n");
        exit(1);
    }
    
    while ((c = fgetc(fp)) != EOF)
        putchar(c);
    return;
}

int main(int argc, char* argv[]) {
    for(int i = 1; i < argc; i++) {
        print_file(argv[i]);
    }
    exit(0);
}