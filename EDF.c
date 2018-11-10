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
check_arrivals(pqueue *rdqueue, int cur_time, int nproc)
{
    for(int i = 0; i < nproc; i++)
    {
        if(global_tasks[i]->job_list == NULL && global_tasks[i]->deadline/*period*/ <= cur_time)
        {
            global_tasks[i]->deadline += global_tasks[i]->deadline;
            process *p = process_init(pid_count++, global_tasks[i]->wcet, global_tasks[i]->period, 
                                global_tasks[i]->task_id, global_tasks[i]);
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
                FILE *log_file = fopen("sched-op-edf.txt", "a+");
                fprintf(log_file, "task: %d pid:%d aet: %d\n", cur_proc->task_id, cur_proc->pid, cur_proc->aet);
                fclose(log_file);
                pqueue_extract_process(rdqueue, cur_proc);
                //unlink from job lists
                remove_job(cur_proc->task_ref, cur_proc);
            }
                
        }
        //execute for 1 cycle
        cur_time++;
        //once ret == et change the deadline to + hyperperiod
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
        process *p = process_init(pid_count++, wcet, period, task_count, t);
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
    schedule_edf(ready_queue, task_count, get_lcm());
    return 0;
}