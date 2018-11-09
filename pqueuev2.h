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
    int wcet;
    int period;
    int deadline;
    //this will just be 1 job per task at a given instance in the ready queue
    jobs *job_list;
};

struct process{
    int pid;
    //to map the job to task
    int task_id;
    //actual execution time of this job
    int aet;
    //apriori known as a part of the temporal parameters of the tasks.
    int wcet;
    //current priority : @t=0- assigned; @t=t- current
    int priority;
    //used in sched policies dynamically assigning current prio based upon slack
    int slack;
};

typedef struct pqueue pqueue;
struct pqueue{
    process **ready;
    //depends upon the initial task submission
    int pq_capacity;
    //jobs in ready queue
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