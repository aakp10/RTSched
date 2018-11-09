#include <stdio.h>
#include <stdlib.h>
#include "pqueuev2.h"

static int
get_rchild(pqueue *rdqueue, int root_index)
{
    return (2*root_index + 2);
}

static int
get_lchild(pqueue *rdqueue, int root_index)
{
    return (2*root_index + 1);
}

static int
get_parent(pqueue *rdqueue, int child_index)
{
    return (child_index - 1)/2;
}

static int
get_insert_pos(pqueue *rdqueue)
{
    return rdqueue->pq_size;
}

static void
swap(process **a, process **b)
{
    process *temp = *a;
    *a = *b;
    *b = temp;
}

static void
bubble_up(pqueue *rdqueue)
{
    int parent_index;
    int process_count = rdqueue->pq_size;
    int cur_index = process_count;
    while(cur_index >0)
    {
        parent_index = get_parent(rdqueue, cur_index);
        process *parent = rdqueue->ready[parent_index];
        process *curr = rdqueue->ready[cur_index];
        if(parent->priority > curr->priority)
        {
            swap(rdqueue->ready + parent_index, rdqueue->ready + cur_index);
            cur_index = parent_index;
        }
        else 
            return;       
    }
}

void
pqueue_insert_process(pqueue *rdqueue, process *p)
{
    //search in rdqueue
    for(int i = 0; i < rdqueue->pq_size; i++)
        if(rdqueue->ready[i]->pid == p->pid)
            return;

    int ins_pos = get_insert_pos(rdqueue);
    if(ins_pos == rdqueue->pq_capacity ) {
        perror("Process limit exceeded\n");
        return;
    }
    rdqueue->ready[ins_pos] = p;
    bubble_up(rdqueue);
    rdqueue->pq_size++;
}

process*
pqueue_get_max(pqueue *rdqueue)
{
    process *ret_process;
    return (ret_process = rdqueue->ready[0]) == NULL? NULL : ret_process;
}

static int
get_priority_at_index(pqueue *rdqueue, int index)
{
    return rdqueue->ready[index]->priority;
}

static void
pqueue_heapify(pqueue *rdqueue, int index)
{
    //get the max priority (numerically min) amongst the parent, rchild, lchild
    int rchild_index, lchild_index, smallest;
    smallest = index;
    rchild_index = get_rchild(rdqueue, index);
    lchild_index = get_lchild(rdqueue, index);
    if(rchild_index < rdqueue->pq_size && get_priority_at_index(rdqueue, rchild_index) < get_priority_at_index(rdqueue, smallest))
        smallest = rchild_index;
    if(lchild_index < rdqueue->pq_size && get_priority_at_index(rdqueue, lchild_index) < get_priority_at_index(rdqueue, smallest))
        smallest = lchild_index;
    if(smallest != index) {
        swap(rdqueue->ready + smallest, rdqueue->ready + index);
        pqueue_heapify(rdqueue, smallest);
    }
}

void
pqueue_extract_process(pqueue *rdqueue, process *p)
{
    int process_count = rdqueue->pq_size;
    if(process_count == 0)
        return;
    if(process_count == 1) {
        rdqueue->pq_size--;
        rdqueue->ready[0] = NULL;
        return;
    }
    rdqueue->ready[0] = rdqueue->ready[process_count - 1];
    rdqueue->pq_size--;
    pqueue_heapify(rdqueue, 0);
}

void
pqueue_dec_priority(pqueue *rdqueue, process *p, int index, int delta)
{
    p->priority += delta;
    pqueue_heapify(rdqueue, index);
}

void
pqueue_display_process(pqueue *pq)
{
    printf("READY PROCESSES\n");
    if(pqueue_get_max(pq))
        printf("HIGHEST PRIORITY pid:%d deadline:%d\n", pqueue_get_max(pq)->pid, pqueue_get_max(pq)->deadline);
    for(int i = 0; i < pq->pq_size; i++)
    {
        printf("pid:%d deadline:%d\n", pq->ready[i]->pid, pq->ready[i]->deadline);
    }
    printf("END\n");

}

int
get_aet(int wcet)
{
    //simulation hack to generate actual execution time between 0 and wcet
    //aet is biased towards wcet/2 + rand()
    //seed set depending upon the current paramaters when the job is submitted to ready queue
    srand(time(&cur_time));
    return (rand() % (wcet/2 + 1)) + wcet/2;
}

process*
process_init(int pid_v, int wcet_v, int priority_v, int task_id)
{
    time_t cur_time;
    process *temp = (process*)malloc(sizeof(process));
    temp->task_id = task_id;
    temp->pid = pid_v;
    temp->wcet = wcet_v;
    temp->priority = priority_v;
    // NOTE: Sorry but FUTURE can't be predicted.
    // hence, resort to a random hacky actual exec time generator.
    temp->aet = get_aet();
    return temp;
}

pqueue *
pqueue_init(int process_count, int capacity)
{
    pqueue *pq = (pqueue *)malloc(sizeof(pqueue));
    pq->ready = (process**)malloc(process_count * sizeof(process *));
    pq->pq_capacity = capacity;
    pq->pq_size = 0;
    return pq;
}

task*
task_init(int et, int period, int deadline)
{
    task *temp = (task*)malloc(sizeof(task));
    temp->et = et;
    temp->period = period;
    temp->deadline = deadline;
    temp->job_list = NULL;
}

void
task_submit_job(task *cur_task, process *proc)
{
    jobs *temp_job = (jobs*)malloc(sizeof(jobs));
    temp_job->cur_proc = proc;
    temp_job->next_job = cur_task->job_list;
    cur_task->job_list = temp_job;
}