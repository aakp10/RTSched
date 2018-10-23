#include <stdio.h>
#include <stdlib.h>
#include "pqueue.h"

#define MAXPROCESS 100
process **global_processes;

int
get_gcd(int num1, int num2)
{
    if(num2 == 0)
        return num1;
    return get_gcd(num2, num1%num2);
}

int get_lcm(int nproc)
{
    int hyper_period = 1;
    for(int i = 0; i < nproc; i++)
    {
        process *p2 = global_processes[i];
        //printf("%d, ", p2->period);
        hyper_period = (hyper_period * p2->period)/get_gcd(hyper_period, p2->period);
    }
    return hyper_period;
}

static void
update_slack(pqueue *rdqueue, int nproc, int cur_time)
{
    int prev_slack;
    //printf("UPDATION\n");
    for(int i = 0; i < nproc; i++)
    {
        process *cur_proc = global_processes[i];
        prev_slack = cur_proc->slack;
        
        cur_proc->slack = cur_proc->deadline - cur_time - cur_proc->ret;
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
    int prev_pid = -1;
    while(cur_time <= hyperperiod)
   {
       process *cur_proc = pqueue_get_max(rdqueue);
       if(cur_proc->pid != prev_pid) {
            printf("time:%d process executing: %d\n", cur_time, cur_proc->pid);
            prev_pid = cur_proc->pid;
        }
        cur_time++;
        cur_proc->ret--;
        if(cur_proc->ret == 0) {
            cur_proc->ret = cur_proc->et;
            cur_proc->deadline += cur_proc->period + (cur_proc->period - cur_proc->deadline);
        }
        //update slacks
        update_slack(rdqueue, nproc, cur_time);   
   } 
}

int main()
{
    int process_count;
    int i = 0;
    scanf("%d",&process_count);
    pqueue *ready_queue = pqueue_init(process_count, MAXPROCESS);
    global_processes = (process**)malloc(process_count * sizeof(process*));
    while(i < process_count)
    {
        int pid, et, period, deadline;
        scanf("%d %d %d", &et, &period, &deadline);
        int slack = deadline - et;
        process *p = process_init(i+1, et, period, deadline, slack);
        global_processes[i] = p;
        pqueue_insert_process(ready_queue, p);
        i++;
    }
    pqueue_display_process(ready_queue);
    printf("%d", get_lcm(process_count));
    schedule_lst(ready_queue, process_count, get_lcm(process_count));
    return 0;
}