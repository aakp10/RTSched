#ifndef _PQUEUE_H_
#define _PQUEUE_H_

#define MAX_PROCESSES 100
typedef struct process process;
struct process{
    int pid;
    int et;
    int ret;
    int period;
    int deadline;
    int priority;
    int slack;
};

typedef struct pqueue pqueue;
struct pqueue{
    //note this
    process **ready;
    int pq_capacity;
    int pq_size;
};

//insert
void pqueue_insert_process(pqueue *rdqueue, process *p);
//delete
void pqueue_extract_process(pqueue *rdqueue, process *p);
//decrease priority
void pqueue_dec_priority(pqueue *rdqueue, process *p, int index, int delta);
//getmax
process *pqueue_get_max(pqueue *rdqueue);
process *process_init(int pid_v, int et_v, int period_v, int deadline_v, int priority_v);
pqueue *pqueue_init(int process_count, int capacity);
void pqueue_display_process(pqueue *pq);
#endif