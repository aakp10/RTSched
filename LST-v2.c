#include <stdio.h>
#include <stdlib.h>
#include "pqueuev2.h"

#define MAXPROCESS 100
task **global_tasks;
static int pid_count = 0;
static int task_count = 0;

struct arrival_list{
    int arr_time;
    struct arrival_list *next;
    struct arrival_list *before;
};

static struct arrival_list* anticipated_arrival;

struct arrival_list*
arrival_list_find_position(int arr_time)
{
    struct arrival_list *head = anticipated_arrival;
    while(head && head->next)
    {
        if(head->arr_time >= arr_time)
            return head->before;
        head = head->next;
    }
    return head;
}

void
arrival_list_add(int arr_time)
{
    struct arrival_list *temp_arrival = (struct arrival_list*)malloc(sizeof(struct arrival_list));
    temp_arrival->arr_time = arr_time;
    struct arrival_list *head = arrival_list_find_position(arr_time);
    if (head) {
        temp_arrival->next = head;
        if(head->arr_time >= arr_time) {
        temp_arrival->before = head->before;
            temp_arrival->next = head;
            temp_arrival->before = head->before;
        }
        else {
            temp_arrival->next = head->next;
            head->next = temp_arrival;
            temp_arrival->before = head; 
        }
    }
    else {
        anticipated_arrival = temp_arrival;
        temp_arrival->next = head;
        temp_arrival->before = NULL;
    }
}

int
get_next_arrival()
{
    return anticipated_arrival? anticipated_arrival->arr_time: (1<<30) - 1;
}

void
remove_next_arrival()
{
    if(anticipated_arrival)
    anticipated_arrival = anticipated_arrival->next;
}

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

static void
check_arrivals(pqueue *rdqueue, int cur_time, int nproc)
{
    for(int i = 0; i < nproc; i++)
    {
        if(global_tasks[i]->job_list == NULL && global_tasks[i]->next_release_time/*period*/ <= cur_time)
        {
            //global_processes[i]->ret = global_processes[i]->et;
            //update release time
            global_tasks[i]->next_release_time += global_tasks[i]->period;
            //deadline is considered same as period
            global_tasks[i]->deadline += global_tasks[i]->period;
            /*
            global_tasks[i]->deadline += global_tasks[i]->period;
            */
            //recreate the process .
            process *p = process_init(pid_count++, global_tasks[i]->wcet,
                             global_tasks[i]->period, global_tasks[i]->task_id, global_tasks[i]);
            //enqueue into the job_list
            task_submit_job(global_tasks[i], p);
            pqueue_insert_process(rdqueue, p);
        }
    }
    //pqueue_display_process(rdqueue);
}

void
schedule_lst(pqueue *rdqueue, int nproc, int hyperperiod)
{
    int cur_time = 0;
    int prev_task_id = -1;
    int cur_task_id = -1;
    while(cur_time <= hyperperiod)
    {
        /**
        * FIXME: new arrivals
        */
        check_arrivals(rdqueue, cur_time, nproc);
        process *cur_proc = pqueue_get_max(rdqueue);
        cur_task_id = cur_proc->task_id;
        if(cur_proc) {
            printf("time:%d process executing: %d\n", cur_time, cur_proc->pid);
            //find the next least slack time job
            process *next_proc = get_next_child(rdqueue, 0);
            //Processor claimed by the job for ∆ = next-min-slack-time - current-slack-time.
            int next_cpu_burst;
            if(next_proc) {
                next_cpu_burst = next_proc->slack - cur_proc->slack + 1;
            }
            else {
                next_cpu_burst = cur_proc->ret;
            }
            
            int next_completion = cur_time + next_cpu_burst;
            //check if any new arrivals before consuming this cpu burst
            int next_arrival = get_next_arrival();
            printf("next completion %d", next_completion);
            printf("next arrival %d \n", next_arrival);
            if (next_completion < next_arrival) {
                cur_proc->ret -= next_cpu_burst;
                cur_time += next_cpu_burst;
            }
            else {
                remove_next_arrival();
                cur_proc->ret -= next_arrival - cur_time;
                cur_time = next_arrival;
            }
            //cur_time++;
            
            if(cur_proc->ret == 0) {
                FILE *log_file = fopen("sched-op-lst.txt", "a+");
                int response_time = cur_time - cur_proc->task_ref->next_release_time - cur_proc->task_ref->period; 
                fprintf(log_file, "task: %d pid:%d aet: %d RESPONSE TIME: %d ", cur_proc->task_id, cur_proc->pid,
                            cur_proc->aet, response_time);
                fclose(log_file);
                //updated anticipated_arrival list
                arrival_list_add(cur_proc->task_ref->next_release_time);
                pqueue_extract_process(rdqueue, cur_proc);
                //unlink from job lists
                remove_job(cur_proc->task_ref, cur_proc);
            }
        }
        else
            cur_time++;
        FILE *log_file = fopen("sched-op-lst.txt", "a+");
            fprintf(log_file, "cache impact: %d", check_cache_impact(cur_task_id, prev_task_id));
        fclose(log_file);
        prev_task_id = cur_task_id;
        //update slacks
        update_slack(rdqueue, rdqueue->pq_size, cur_time);   
    }
}

pqueue *
submit_processes()
{
    FILE *task_file = fopen("tasks", "r");
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
        //process_init(int pid_v, int wcet_v, int priority_v, int task_id, task *task_ref);
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
    anticipated_arrival = NULL;
    schedule_lst(ready_queue, task_count, get_lcm());
    return 0;
}