#include <stdio.h>
#include "pqueue.h"

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
bubble_up(pqueue *rdqueue)
{
    int parent_index;
    int process_count = rdqueue->pq_size;
    int cur_index = process_count;
    while(cur_index > -1)
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
insert_process(pqueue *rdqueue, process *p)
{
    int ins_pos = get_insert_pos(rdqueue);
    if(ins_pos == rdqueue->pq_capacity ) {
        perror("Process limit exceeded\n");
        return;
    }
    rdqueue->ready[ins_pos] = p;
    rdqueue->pq_size++;
    bubble_up(rdqueue);
}

process*
get_max(pqueue *rdqueue)
{
    process *ret_process;
    return (ret_process = rdqueue->ready[0]) == NULL? NULL : ret_process;
}

static void
heapify(pqueue *rdqueue, int index)
{
    //get the max priority (numerically min) amongst the parent, rchild, lchild
    int rchild_index, lchild_index, smallest;
    smallest = index;
    rchild_index = get_rchild(rdqueue, index);
    lchild_index = get_lchild(lchild_index, index);
    if(get_priority_at_index(rdqueue, rchild_index) < get_priority_at_index(rdqueue, smallest))
        smallest = rchild_index;
    if(get_priority_at_index(rdqueue, lchild_index) < get_priority_at_index(rdqueue, smallest))
        smallest = lchild_index;
    if(smallest != index) {
        swap(rdqueue->ready + smallest, rdqueue->ready + index);
        heapify(rdqueue, smallest);
    }
}

void
extract_process(pqueue *rdqueue, process *p)
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
    heapify(rdqueue, 0);
}


