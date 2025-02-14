#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


// do not run, will crash...
void unzip_file(char* filename) {
    FILE* fp = fopen(filename, "r");
    int rep_count = 0;
    // char character_buffer[1]; 
    int c;

    if(!fp) {
        printf("wunzip: cannot open file\n");
        exit(1);
    }

    int counter = 0;
    while((c = fgetc(fp)) != EOF) {
        if(counter < 4) {
            rep_count += c << (counter*8);
            counter++;
        }
        else {
            for(int i = 0; i < rep_count; i++) {
                fputc(c, stdout);
            }
            counter = 0;
            rep_count = 0;
        }
    }

}

int main(int argc, char* argv[]) {
    if(argc == 1) {
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    for(int i  = 1; i < argc; i++) {
        unzip_file(argv[i]);
    }

}
