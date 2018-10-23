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

void
schedule_edf(pqueue *rdqueue, int nproc, int hyperperiod)
{
    //execute until 1 hyperperiod
    int cur_time = 0;
    int prev_pid = -1;
    while(cur_time <= hyperperiod)
   {
       process *cur_proc = pqueue_get_max(rdqueue);
       if(cur_proc->pid != prev_pid) {
            printf("time:%d process executing: %d\n", cur_time, cur_proc->pid);
            prev_pid = cur_proc->pid;
        }
        //insert release time all the getmax priority 
        //execute for 1 cycle
        cur_time++;
        //change ret
        cur_proc->ret--;
        if(cur_proc->ret == 0)
        {
            cur_proc->ret =  cur_proc->et;
            pqueue_dec_priority(rdqueue, cur_proc, 0, cur_proc->period + (cur_proc->period - cur_proc->deadline));
        }
        //once ret == et change the deadline to + hyperperiod
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
        process *p = process_init(i+1, et, period, deadline);
        global_processes[i] = p;
        pqueue_insert_process(ready_queue, p);
        i++;
    }
    pqueue_display_process(ready_queue);
    printf("%d", get_lcm(process_count));
    schedule_edf(ready_queue, process_count, get_lcm(process_count));
    return 0;
}