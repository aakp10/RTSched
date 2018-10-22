#ifndef _PQUEUE_H_
#define _PQUEUE_H_

#define MAX_PROCESSES 100
typedef struct process process;
struct process{
    int pid;
    int et;
    int period;
    int deadline;
    int priority;
};

typedef struct pqueue pqueue;
struct pqueue{
    //note this
    process **ready;
    int pq_capacity;
    int pq_size;
};

//insert
void insert_process(pqueue *rdqueue, process *p);
//delete
void extract_process(pqueue *rdqueue, process *p);
//decrease priority
void dec_priority(pqueue *rdqueue, process *p);
//getmax
process *get_max(pqueue *rdqueue);
process *process_init(int pid_v, int et_v, int period_v, int deadline_v);
pqueue *pqueue_init(int process_count, int capacity);
#endif