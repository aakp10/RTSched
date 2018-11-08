#ifndef _PQUEUE_H_
#define _PQUEUE_H_
 
#define MAX_PROCESSES 100
typedef struct process process;
//FIXME task struct
typedef struct jobs jobs;
struct jobs{
    process *cur_proc;
    jobs *next_job;
};

typedef struct task task;
struct task{
    int et;
    int period;
    int deadline;
    jobs *job_list;
};

struct process{
    int pid;
    int task_id;
    //FIXME int wcet;
    int aet;
    int wcet;
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
process *process_init(int pid_v, int et_v, int priority_v, int task_id);
pqueue *pqueue_init(int process_count, int capacity);
void pqueue_display_process(pqueue *pq);
void task_submit_job(task *cur_task, process *proc);
task *task_init(int et, int period, int deadline);
#endif