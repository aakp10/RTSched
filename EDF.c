#include <stdio.h>
#include <stdlib.h>
#include "pqueue.h"

#define MAXPROCESS 100
process **global_processes;


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
    return 0;
}