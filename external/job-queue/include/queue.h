/*
 * This file defines a blocking concurrent job queue API.
 * 
 * It stores generic job structures in order without breaking down
 * when multiple threads tries to add jobs or get jobs at the same time.
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "errcode.h"
#include <stdbool.h>
#include <stddef.h>

/*
 * This type definition creates a job_func type that represents
 * a generic function.
 */
typedef void *(*job_func)(void *);

/*
 * This structure represents a job that consists of
 * a function and argument for that function.
 */
typedef struct job job_t;

/*
 * job_init: Initializes a job with job_func, and arguments.
 *           Allocates new memory for job object.
 *
 *           On  success, returns QUEUE_OK. Otherwise returns an error code.
 *
 *           If function fails, the job value og *job will not be changed.
 *
 *           The ownership of the structure belongs to the API. Caller must not
 *           call free. Caller should call 'job_destroy' to destroy the job object.
 */
job_queue_return_code_t job_init(job_t **job, job_func fn, void *args);

/*
 * job_destroy: Destroys and releases the memory allocated for the job object.
 *              Changes the value of the job pointer to NULL.
 */
void job_destroy(job_t **job);

/*
 * job_execute: Execute the function represented with given job.
 *              If job or job function is NULL, return value is NULL.
 *              Otherwise return value if the return value of the job function.
 * */
void *job_execute(const job_t *job);

/*
 * This structure represents a queue that stores
 * jobs in order and manages them.
 */
typedef struct job_queue job_queue_t;

/*
 * job_queue_init: Initializes a job queue pointer.
 *                 Allocates memory for the queue and its job list.
 *
 *                 On success, returns QUEUE_OK. Otherwise returns an error code.
 *
 *                 If function fails, job queue pointer will not be changed.
 *
 *                 The ownership of the job queue structure is belongs to the API.
 *                 Caller must not call free on the job queue struct. Caller should
 *                 call 'job_queue_destroy' to destroy the queue object.
 */
job_queue_return_code_t job_queue_init(job_queue_t **queue,
                                     size_t capacity);

/*
 * job_queue_destroy: Release the memory allocated for given queue.
 *                    After makes the job_queue pointer NULL.
 *
 *                    All producer and consumer threads must be done their jobs
 *                    when this function has called.
 */
void job_queue_destroy(job_queue_t **queue);

/*
 * job_queue_push: If queue is not full, immediately adds
 *                 the given job to the end of the queue.
 *                 
 *                 If queue is full, waits until it is not
 *                 and adds the job after that.
 *                 
 *                 On success, returns QUEUE_OK. Otherwise
 *                 returns an error code.
 *
 *                 Does take the ownership of the job object.
 *                 Caller shouldn't destroy the job object in a queue.
 */
job_queue_return_code_t job_queue_push(job_queue_t *queue,
                                       job_t *job);

/*
 * job_queue_pop: If queue is not empty, gets the next job and removes it.
 *
 *                If queue is empty, waits until it gets
 *                unempty and gets the next job and removes it.
 *                
 *                On success, 'job' variable will be equal
 *                to the next job in the list and QUEUE_OK is returned.
 *                Otherwise returns an error code.
 *
 *                Gives the ownership of the job object to the caller.
 *                Caller should destroy the job object.
 */
job_queue_return_code_t job_queue_pop(job_queue_t *queue, job_t **job);
#endif
