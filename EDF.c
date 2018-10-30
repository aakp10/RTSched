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
check_arrivals(pqueue *rdqueue, int cur_time, int nproc)
{
    for(int i = 0; i < nproc; i++)
    {
        if(!global_processes[i]->ret && global_processes[i]->deadline/*period*/ <= cur_time)
        {
            global_processes[i]->ret = global_processes[i]->et;
            global_processes[i]->deadline += global_processes[i]->deadline;
            process *p = process_init(i+1, global_processes[i]->et, global_processes[i]->period, 
                                global_processes[i]->deadline, global_processes[i]->period);
            //FIXME :recreate the process this is reinsertion.
            pqueue_insert_process(rdqueue, p);
        }
    }
    //pqueue_display_process(rdqueue);
}

void
schedule_edf(pqueue *rdqueue, int nproc, int hyperperiod)
{
    //execute until 1 hyperperiod
    int cur_time = 0;
    int prev_pid = -1;
    while(cur_time <= hyperperiod)
   {
        check_arrivals(rdqueue, cur_time, nproc);
        process *cur_proc = pqueue_get_max(rdqueue);
        if(cur_proc) {
            printf("time:%d process executing: %d\n", cur_time, cur_proc->pid);
            //insert release time all the getmax priority 
            //change ret
            cur_proc->ret--;
            if(cur_proc->ret == 0)
            {
                global_processes[cur_proc->pid - 1]->ret = 0;
                pqueue_extract_process(rdqueue, cur_proc);
            }
                
        }
        //execute for 1 cycle
        cur_time++;
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
        process *p = process_init(i+1, et, period, deadline, deadline);
        global_processes[i] = p;
        pqueue_insert_process(ready_queue, p);
        i++;
    }
    pqueue_display_process(ready_queue);
    printf("%d", get_lcm(process_count));
    schedule_edf(ready_queue, process_count, get_lcm(process_count));
    return 0;
}