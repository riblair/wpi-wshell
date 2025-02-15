#include "scheduler.h"

struct args* arg_parse(char* argv[]) {
    struct args* args = malloc(sizeof(struct args));
    for(int i = 1; i < 5; i++) {
        switch (i)
        {
        case 1: // analysis mode
            args->analysis_mode = atoi(argv[i]);
            break;
        case 2: // policy type
            strncpy(args->policy, argv[i], 5);
            break;
        case 3: // input file_path
            strncpy(args->file_path, argv[i], 100);
            break;
        case 4: // analysis mode
            if(atoi(argv[i])) args->timeslice = atoi(argv[i]);
            else args->timeslice = 1;
            break;
        }
    }
    return args;
}

struct job* create_job_queue(char* file_path) {
    
    FILE* in_file = fopen(file_path, "r");
    if(in_file == NULL) return NULL;
    char* line = malloc(100* sizeof(char));
    size_t len = 0;
    __ssize_t nread;
    int ids = 0;


    struct job* job_head = malloc(sizeof(struct job));
    struct job* job_iter = job_head;
    struct job* job_iter2;

    while((nread = getline(&line, &len, in_file)) != -1) {
        job_iter2 = job_iter;
        job_iter->id = ids++;
        job_iter->arrival_time = atoi(strtok(line, ","));
        job_iter->length = atoi(strtok(NULL, ","));

        job_iter->next = malloc(sizeof(struct job));
        job_iter = job_iter->next;
    }
    free(job_iter);
    job_iter2->next = NULL;
    return job_head;
}

int count_jobs(struct job* job_head) {
    int count = 0;
    struct job* job_iter = job_head;

    while(job_iter != NULL) {
        count++;
        job_iter = job_iter->next;
    }

    return count;
}

void handler_FIFO(struct job* job_head, struct metrics* run_metrics) {

    // iterate through jobs in order, runnning them for their entire runtime

    int sim_time = 0;
    int end_cond = 0;
    struct job* job_iter = job_head;

    while(!end_cond) {
        if(sim_time < job_iter->arrival_time) {
            sim_time = job_iter->arrival_time;
        }
        // run time for selected amount
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
            sim_time, job_iter->id, job_iter->arrival_time, job_iter->length);
        fflush(stdout);
        sim_time+= job_iter->length;
        
        job_iter = job_iter->next;
        // end cond is we are out of jobs
        end_cond = (job_iter == NULL);
    }
}

void handler_SJF(struct job* job_head, struct metrics* run_metrics) {
    //TODO: implement
    return;
}

void handler_RR(struct job* job_head, struct metrics* run_metrics) {
    //TODO: implement
    return;
}

void analyze_run(struct metrics* run_metrics) {
    //TODO: implement
    return;
}

int main(int argc, char* argv[]) {
    // four args on input
    /*
    Four inputs on cmd line
    1. analysis mode -> 0/1 disable/enable
    2. name of scheduler {"FIFO", "SJF", "RR"}
    3. name of workload file ".in files"
    4. timeslice duration...
    
    jobs are listed in order of arrival,
    ordered as pairs (arrival-time, run-time)
    all times are integers

    jobs should have ids based on arrival, and be stored in a linked-list.

    Should simulate execution and print out arrival times, and run times
    "t=0: [Job 0] arrived at [0], ran for: [3]"
    */

    // parse inputs from user
    // parse .in file to LL
    // start execution based on policy
    // monitor results
    // profit?

    if(argc != 5) {
        printf("Incorrect amount of args inputs. Expected ./scheduler [0/1] [policy {'FIFO', 'SJF', 'RR'}] ['input_file_path.in'] [timeslice duration]");
        exit(1);
    }
    struct args* args = arg_parse(argv);
    struct job* job_head = create_job_queue(args->file_path);
    struct metrics* run_metrics = malloc(sizeof(struct metrics));
    run_metrics->num_jobs = count_jobs(job_head);
    
    void (*policy_func)(struct job*, struct metrics*);
    
    if(!strncmp("FIFO", args->policy, 4)) policy_func = handler_FIFO;
    else if(!strncmp("SJF", args->policy, 3)) policy_func = handler_SJF;
    else if(!strncmp("RR", args->policy, 2)) policy_func = handler_RR;
    else {
        printf("UNEXPECTED POLICY");
        exit(1);
    }
    printf("Execution trace with %s:\n", args->policy);
    policy_func(job_head, run_metrics);
    printf("End of execution with %s.\n", args->policy);
    if(args->analysis_mode) analyze_run(run_metrics);
    return 0;
}
