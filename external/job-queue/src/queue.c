/*
 * This file implements functions defined in 'include/queue.h'
 */

#include "../include/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct job {
        job_func job_function; // function to do the job
        void *args;            // arguments to the function
};

job_queue_return_code_t job_init(job_t **job, job_func fn, void *args) {
        if (job == NULL) return QUEUE_NULL_ARGUMENT;

        job_t *j = malloc(sizeof(job_t));
        if (j == NULL) {
                return QUEUE_MEMORY_ERROR;
        }
        
        j->job_function = fn;
        j->args = args;
        
        *job = j;

        return QUEUE_OK;
}

void job_destroy(job_t **job) {
        if (job == NULL || *job == NULL) return;

        free(*job);

        *job = NULL;
}

void *job_execute(const job_t *job) {
        if (job == NULL) return NULL;
        if (job->job_function == NULL) return NULL;

        return job->job_function(job->args);
}

struct job_queue {
        job_t **jobs;           // list to store job pointers
        size_t capacity;       // capacity of job list
        size_t size;           // current size of job list
        size_t front;                   // The index of the first element
        size_t rear;                    // The index of the last element
        pthread_mutex_t mutex; // mutex to prevent multiple threads
                               // to reach the job_queue fields at the same time.
        pthread_cond_t cond_empty;   // Condition variable that keeps threads sleeping
                                     // while queue is empty
        pthread_cond_t cond_full;    // Condition variable that keeprs threads sleeping
                                     // while queue is full.
};

job_queue_return_code_t job_queue_init(job_queue_t **queue,
                                       size_t capacity) {
        if (queue == NULL || capacity == 0) return QUEUE_NULL_ARGUMENT;

        // Allocate a job queue object
        job_queue_t *q = malloc(sizeof(job_queue_t));
        if (q == NULL) {
                return QUEUE_MEMORY_ERROR;
        }
        
        q->jobs = malloc(sizeof(job_t *) * capacity);
        if (q->jobs == NULL) {
                free(q);
                return QUEUE_MEMORY_ERROR;
        }
        
        if (pthread_mutex_init(&q->mutex, NULL) != 0) {
                free(q->jobs);
                free(q);

                return QUEUE_PTHREAD_ERROR;
        }

        if (pthread_cond_init(&q->cond_full, NULL) != 0) {
                free(q->jobs);
                pthread_mutex_destroy(&q->mutex);
                free(q);
                
                return QUEUE_PTHREAD_ERROR;
        }

        if (pthread_cond_init(&q->cond_empty, NULL) != 0) {
                free(q->jobs);
                pthread_mutex_destroy(&q->mutex);
                pthread_cond_destroy(&q->cond_empty);
                free(q);
                
                return QUEUE_PTHREAD_ERROR;
        }
        
        q->capacity = capacity;
        q->size = 0;
        q->front = 0;
        q->rear = 0;

        // Everything is OK, set the argument
        *queue = q;
                
        return QUEUE_OK;
}

void job_queue_destroy(job_queue_t **queue) {
        if (queue == NULL || *queue == NULL) return;
        job_queue_t *q = *queue;
        
        while (q->size > 0) {
                job_destroy(&q->jobs[q->front]);
                
                q->front = (q->front + 1) % q->capacity;
                q->size--;
        }

        if (q->jobs) free(q->jobs);

        pthread_mutex_destroy(&q->mutex);
        pthread_cond_destroy(&q->cond_empty);
        pthread_cond_destroy(&q->cond_full);
        free(q);
        
        // Make the pointer NULL.
        *queue = NULL;
}

/*
 * job_queue_empty: Returns true if there is no jobs in the job queue.
 *                  Otherwise returns false.
 *
 *                  If given pointer is NULL, returns true.
 */
static bool job_queue_empty(job_queue_t *queue) {
        if (queue == NULL) return true;

        bool r = queue->size == 0;

        return r;
}

/*
 * job_queue_full: Returns true if the job queue is full.
 *                 Otherwise returns false.
 *
 *                 If given pointer is NULL, returns false.
 */
static bool job_queue_full(job_queue_t *queue) {
        if (queue == NULL) return false;

        bool r = queue->size == queue->capacity;

        return r;
}

job_queue_return_code_t job_queue_push(job_queue_t *queue,
                                       job_t *job) {
        if (queue == NULL) return QUEUE_NULL_ARGUMENT;

        // while queue is full, sleep and wait.
        pthread_mutex_lock(&queue->mutex);

        while (job_queue_full(queue)) {
                printf("[Producer %lu] waiting for adding jobs...\n", pthread_self());
                pthread_cond_wait(&queue->cond_full, &queue->mutex);
        }
        
        // now add the job to the queue
        queue->jobs[queue->rear] = job;
        queue->rear = (queue->rear + 1) % queue->capacity;
        if (queue->size == 0) {
                queue->front = 0;
        } 
        queue->size++;
        
        // send a signal that queue is not empty
        pthread_cond_signal(&queue->cond_empty);

        pthread_mutex_unlock(&queue->mutex);

        return QUEUE_OK;
}

job_queue_return_code_t job_queue_pop(job_queue_t *queue, job_t **job) {
        if (queue == NULL || job == NULL) {
                return QUEUE_NULL_ARGUMENT;
        }
        
        // wait until queue is empty.
        pthread_mutex_lock(&queue->mutex);

        while (job_queue_empty(queue)) {
                printf("[Worker %lu] waiting for getting jobs...\n", pthread_self());
                pthread_cond_wait(&queue->cond_empty, &queue->mutex);
        }

        // get the next job.
        job_t *next_job = queue->jobs[queue->front];
        queue->jobs[queue->front] = NULL;
        queue->front = (queue->front + 1) % queue->capacity;
        queue->size--;
        
        // send a signal to the threads that are waiting
        // the queue is full.
        pthread_cond_signal(&queue->cond_full);

        if (queue->size == 0) {
                queue->front = 0;
                queue->rear = 0;
        }

        pthread_mutex_unlock(&queue->mutex);
        
        // Operation successfull, modify 'job'.
        *job = next_job;

        return QUEUE_OK;
}


