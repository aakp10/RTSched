#include <stdio.h>
#include <stdlib.h>
#include "pqueuev2.h"

#define MAXPROCESS 100
task **global_tasks;
static int pid_count = 0;
static int task_count = 0;
int max_prio_next_release = 1<<32;

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
    if (anticipated_arrival)
    return anticipated_arrival? anticipated_arrival->arr_time: (1<<31) - 1;
}

void
remove_next_arrival()
{
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
update_max_priority_next_release(task *t)
{
    max_prio_next_release = min(max_prio_next_release, t->next_release_time);
}

static void
check_arrivals(pqueue *rdqueue, int cur_time, int nproc)
{
    for(int i = 0; i < nproc; i++)
    {
        if(global_tasks[i]->job_list == NULL && global_tasks[i]->next_release_time/*period*/ <= cur_time)
        {
            global_tasks[i]->next_release_time += global_tasks[i]->period;
            //deadline is considered same as period
            global_tasks[i]->deadline += global_tasks[i]->deadline;
            /*
            global_tasks[i]->deadline += global_tasks[i]->period;
            */
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
            //sched point at min of arrival or completion
            int next_completion = cur_proc->ret + cur_time;
            int next_arrival = get_next_arrival();
            printf("Next Completion %d Next Arrival %d\n", next_completion, next_arrival);
                       
            if (next_completion < next_arrival) {
                cur_time += cur_proc->ret;
                cur_proc->ret -= cur_proc->ret;
            }
            else {
                remove_next_arrival();
                cur_proc->ret -= next_arrival - cur_time;
                cur_time = next_arrival;
            }
            if(cur_proc->ret == 0)
            {
                FILE *log_file = fopen("sched-op-edf.txt", "a+");
                int response_time = cur_time - cur_proc->task_ref->next_release_time - cur_proc->task_ref->period; 
                fprintf(log_file, "task: %d pid:%d aet: %d RESPONSE TIME: %d\n", cur_proc->task_id, cur_proc->pid,
                            cur_proc->aet, response_time);
                fclose(log_file);
                //updated anticipated_arrival list
                arrival_list_add(cur_proc->task_ref->next_release_time);
                pqueue_extract_process(rdqueue, cur_proc);
                //current max prio job becomes the earliest deadline job
                max_prio_next_release = pqueue_get_max(rdqueue)->task_ref->deadline;
                //update next release time—maybe this gets released again before the complete execution
                //and it needs to be preempted to check the current prio status of deadlines.
                task_update_next_release(cur_proc->task_ref);
                //recalculate the max priority next release time
                update_max_priority_next_release(cur_proc->task_ref);
                //unlink from job lists
                remove_job(cur_proc->task_ref, cur_proc);
            }
                
        }
        //execute for 1 cycle—already handled
        else
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
        process *p = process_init(pid_count++, wcet, deadline, task_count, t);
        max_prio_next_release = min(t->next_release_time, max_prio_next_release);
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
    schedule_edf(ready_queue, task_count, get_lcm());
    return 0;
}