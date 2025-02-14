#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct job {
    int id;
    int length;
    // other such neccesities?
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
void handler_FIFO(struct job*, struct metrics*);
void handler_SJF(struct job*, struct metrics*);
void handler_RR(struct job*, struct metrics*);
void analyze_run(struct metrics*);
