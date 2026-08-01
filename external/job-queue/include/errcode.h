/*
 * This file defines function return codes for job queue.
 */

#ifndef __ERRCODE_H__
#define __ERRCODE_H__

typedef enum job_queue_return_code {
        QUEUE_OK = 0,
        QUEUE_MEMORY_ERROR,
        QUEUE_NULL_ARGUMENT,
        QUEUE_PTHREAD_ERROR
} job_queue_return_code_t;

#endif
