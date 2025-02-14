#include <stdio.h>
#include <stdlib.h>

void outputCurrent(int count, int currentChar){
    fwrite(&count, sizeof(unsigned int), 1, stdout);
    fputc(currentChar, stdout);
}

void zip (char* files[], int numFiles){
    int curChar;
    int prevChar = -1;
    int count = 0;


    for (int i = 0; i < numFiles; i++){
        FILE *file = fopen(files[i], "r");
        if (file == NULL) {
            printf("wzip: cannot open file\n");
            exit(1);
        }
        while ((curChar = fgetc(file)) != -1) {
            if (curChar == prevChar){
                count++;
            }else{
                if (prevChar != -1){
                    outputCurrent(count,prevChar);
                }
                prevChar = curChar;
                count = 1;
            }
        }
        fclose(file);
    }

    if (prevChar != -1) {
        outputCurrent(count, prevChar);
    }

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    zip(&argv[1], argc-1);

    return 0;
}
