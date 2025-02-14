#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct job {
    int id; 
    int length; 
    struct job *next; 
} job;


void FIFO(char file[]){
    int time_start = 0; 
    FILE* f1;
    int arrival;  
    int length; 
    int count = 0; 

    f1 = fopen(file, "r"); 
    printf("Execution trace with FIFO:\n"); 
    while(fscanf(f1, "%d,%d", &arrival, &length) == 2){
        if (time_start < arrival){
            time_start = arrival; 
        }
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", time_start, count, arrival, length); 
        count ++;
        time_start += length; 
    }
    printf("End of execution with FIFO.\n");
}

int main(int argc, char* argv[]){
    if (argc != 5){
        perror("Wrong number of command line arguments!"); 
        exit(1); 
    }

    //bool enable = atoi(argv[1]); 
    char scheduling[10]; 
    strcpy(scheduling, argv[2]);
    char file[25];
    strcpy(file, argv[3]); 
    //int time_slice = atoi(argv[4]); 

    if (strcmp(scheduling, "FIFO") == 0){
        FIFO(file); 
    }
}


