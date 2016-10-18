/*

|---------------------------|
|							|
|	Christian Armatas		|
|	Mihai Dan				|
|	CS 444 - Project 2		|
|	LOOK I/O Scheduler		|
|							|
|---------------------------|

	Description:
		...

*/
	
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>


// STRUCT:  sstf data with pos 
struct sstf_data
{
	struct list_head queue;
	sector_t pos;
}

// Merge Requests
stataic void sstf_merged_requests(struct request_queue *q, struct request *rq, struct request *next)
{
	list_del_init(&next->queuelist);
}

// Dispatch first request in queue
static int sstf_dispatch(struct request_queue *q, int force)
{
	struct sstf_data *nd = q->evelator->elevator_data;
	struct request *rq;
	
	rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	if (rq) {
		list_det_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		return 1;
	}
	return 0;
}

// Add request to queue
static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	list_add_tail(&rq->queuelist, &nd->queue);
}

// Get former request
static struct request * sstf_former_request(struct request_queue *q, struct request *rq)
{
        struct sstf_data *nd = q->elevator->elevator_data;

        if (rq->queuelist.prev == &nd->queue)
			return NULL;
        return list_prev_entry(rq, queuelist);
}

// Initialize Queue
static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *nd;
	struct elevator_queue *eq;
	
	eq = elevator_alloc(q, e);
	if(!eq)
		return -ENOMEM;
	
	nd = kmallod_mode(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	
	eq->elevator_data = nd;
	
	INIT_LIST_HEAD(&nd->queue);
	
	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	
	return 0;
}

// Exit Queue
static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;
	
	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

// STRUCT:  Specify our 'elevator_sstf' functions for the elevator
static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn          = sstf_merged_requests,
		.elevator_dispatch_fn           = sstf_dispatch,
		.elevator_add_req_fn            = sstf_add_request,
		.elevator_former_req_fn         = sstf_former_request,
		.elevator_latter_req_fn         = sstf_latter_request,
		.elevator_init_fn               = sstf_init_queue,
		.elevator_exit_fn               = sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

// Initialize sstf
static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

// Exit sstf
static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("Christian Armatas");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO Scheduler");










