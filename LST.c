#include <stdio.h>
#include <stdlib.h>
#include "pqueuev2.h"

#define MAXPROCESS 100
task **global_tasks;
static int pid_count = 0;
static int task_count = 0;

int
get_gcd(int num1, int num2)
{
    if(num2 == 0)
        return num1;
    return get_gcd(num2, num1%num2);
}

int get_lcm()
{
    int hyper_period = 1;
    for(int i = 0; i < task_count; i++)
    {
        task *p2 = global_tasks[i];
        //printf("%d, ", p2->period);
        hyper_period = (hyper_period * p2->period)/get_gcd(hyper_period, p2->period);
    }
    return hyper_period;
}

static void
update_slack(pqueue *rdqueue, int nproc, int cur_time)
{
    //FIXME
    int prev_slack;
    //printf("UPDATION\n");
    process** ready = rdqueue->ready;
    for(int i = 0; i < nproc; i++)
    {
        process *cur_proc = ready[i];
        prev_slack = cur_proc->slack;
        
        cur_proc->slack = cur_proc->task_ref->deadline - cur_time - cur_proc->ret;
        if(cur_proc->slack != prev_slack)
        {
            pqueue_dec_priority(rdqueue, cur_proc, i, cur_proc->slack - cur_proc->priority);
        }
        //printf("pid:%d priority: %d slack %d \n", cur_proc->pid, cur_proc->priority, cur_proc->slack);
    }
    //printf("END\n");
}

void
schedule_lst(pqueue *rdqueue, int nproc, int hyperperiod)
{
    int cur_time = 0;
    while(cur_time <= hyperperiod)
    {
        /**
        * FIXME: new arrivals
        */

        process *cur_proc = pqueue_get_max(rdqueue);
        if(cur_proc) {
            printf("time:%d process executing: %d\n", cur_time, cur_proc->pid);
            //find the next least slack time job
            process *next_proc = get_next_child(rdqueue, 0);
            //Processor claimed by the job for ∆ = next-min-slack-time - current-slack-time.
            if(next_proc) {
                cur_time += cur_proc->slack - next_proc->slack + 1;
                cur_proc->ret -= cur_proc->slack - next_proc->slack; //check this
            }
            else {
                cur_time += cur_proc->ret;
                cur_proc->ret = 0;

            }
            
            if(cur_proc->ret == 0) {
                FILE *log_file = fopen("sched-op-lst.txt", "a+");
                fprintf(log_file, "task: %d pid:%d aet: %d\n", cur_proc->task_id, cur_proc->pid, cur_proc->aet);
                fclose(log_file);
                pqueue_extract_process(rdqueue, cur_proc);
                //unlink from job lists
                remove_job(cur_proc->task_ref, cur_proc);
            }
        }
        //update slacks
        update_slack(rdqueue, rdqueue->pq_size, cur_time);   
    }
}

pqueue *
submit_processes()
{
    FILE *task_file = fopen("tasks-edf", "r");
    int wcet, period, deadline;
    int task_no = 0;
    int process_count;
    fscanf(task_file,"%d", &process_count);
    pqueue *ready_queue = pqueue_init(process_count, MAXPROCESS);
    global_tasks = (task**)malloc(process_count * sizeof(task*));
    //wcet, period, deadline
    while(fscanf(task_file, "%d, %d, %d", &wcet, &period, &deadline) == 3)
    {
        task *t = task_init(task_count++, wcet, period, deadline);
        process_init(int pid_v, int wcet_v, int priority_v, int task_id, task *task_ref);
        process *p = process_init(pid_count++, wcet, deadline - wcet /*slack at t = 0*/, task_count, t);
        task_submit_job(t, p);
        global_tasks[task_no++] = t;
        pqueue_insert_process(ready_queue, p);
    }
    fclose(task_file);
    return ready_queue;
}

int main()
{
    pqueue *ready_queue = submit_processes();
    pqueue_display_process(ready_queue);
    printf("%d", get_lcm());
    schedule_lst(ready_queue, task_count, get_lcm());
    return 0;
}