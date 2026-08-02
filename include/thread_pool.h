/*
 * This file defines thread pool API.
 *
 */

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <stddef.h>
#include "queue.h"

/*
 * A return code that represents the error or success of a function.
 */
typedef enum thread_pool_return_code {
        THREAD_POOL_OK,
        THREAD_POOL_MEMORY_ERROR,
        THREAD_POOL_PTHREAD_ERROR,
        THREAD_POOL_NULL_ARGUMENT,
        THREAD_POOL_QUEUE_ERROR,
        THREAD_POOL_SHUTDOWNED
} thread_pool_return_code_t;

/*
 * This type definition defines a thread_pool_t type.
 * This represents a thread pool with a blocking concurrent
 * queue with fixed size and fixed number of worker threads.
 */
typedef struct thread_pool thread_pool_t;

/*
 * thread_pool_init: Initiales given thread_pool_t pointer.
 *                   Allocates memory dynamically.
 *                   
 *                   The ownership of the structure belongs to the API
 *                   and caller should call thread_pool_destroy to release
 *                   the memory allocated for the thread pool.
 *                   
 *                   On success, returns THREAD_POOL_OK '*pool' will show the
 *                   thread pool structure. On failure, an error code is returned
 *                   and given pointer will not be changed.
 */
thread_pool_return_code_t thread_pool_init(thread_pool_t **pool,
                                           size_t workers,
                                           size_t queue_cap);

/*
 * thread_pool_submit: Creates a job_t and adds the job to
 *                     the queue in the thread pool.
 *
 *                     On success, returns THREAD_POOL_OK and on failure
 *                     an error code is returned.
 */
thread_pool_return_code_t thread_pool_submit(thread_pool_t *pool, job_func fn, void *args);

/* 
 *  thread_pool_destroy: Releases the memory allocated for the thread pool.
 *                       Also makes the pointer to show NULL.
 */
void thread_pool_destroy(thread_pool_t **thread_pool);

#endif
