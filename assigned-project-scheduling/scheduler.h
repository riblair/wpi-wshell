#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

struct job {
    int id;
    int arrival_time;
    int length;
    // other such neccesities?
    // def need some for metrics...
    struct job *next;
};

struct metrics {
    float num_jobs;
    float sum_run_time;
    float sum_response_time;
};

struct args {
    int analysis_mode;
    char policy[5];
    char file_path[100];
    int timeslice;
};

struct args* arg_parse(char**);
struct job* create_job_queue(char*);
int count_jobs(struct job*);
struct job* handler_FIFO(struct job*, int);
struct job* handler_SJF(struct job*, int);
struct job* handler_RR(struct job*, int);
void handle_run(struct job*, int, struct metrics*, struct job*(struct job*, int));
void analyze_run(struct metrics*);
