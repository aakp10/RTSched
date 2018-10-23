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
    return 0;
}