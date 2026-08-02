#include "thread_pool.h"
#include "queue.h"
#include "errcode.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct thread_pool {
        job_queue_t *queue;    // queue to store and manage jobs
        pthread_t *workers;    // workers to execute the jobs
        size_t worker_count;   // # of workers
        bool shutdown;         // thread pool is terminated or not
        pthread_mutex_t mutex;
};

static void *worker(void *args) {
        thread_pool_t *pool = (thread_pool_t *)args;
        if (pool == NULL) {
                return NULL;
        }

        while (true) {
                job_t *j = NULL;
                job_queue_pop(pool->queue, &j);
                
                if (j == NULL) {
                        break;
                }

                job_execute(j);

                job_destroy(&j);
        }

        return NULL;
}

thread_pool_return_code_t thread_pool_init(thread_pool_t **pool,
                                           size_t workers,
                                           size_t queue_cap) {
        if (pool == NULL) return THREAD_POOL_NULL_ARGUMENT;
        if (workers == 0 || queue_cap == 0) {
                return THREAD_POOL_INVALID_ARGUMENT;
        }

        *pool = malloc(sizeof(thread_pool_t));
        if (*pool == NULL) {
                return THREAD_POOL_MEMORY_ERROR;
        }

        (*pool)->workers = malloc(sizeof(pthread_t) * workers);
        if ((*pool)->workers == NULL) {
                free(*pool);
                return THREAD_POOL_MEMORY_ERROR;
        }
        
        if (pthread_mutex_init(&(*pool)->mutex, NULL) != 0) {
                free((*pool)->workers);
                free(*pool);
                return THREAD_POOL_PTHREAD_ERROR;
        }

        if (job_queue_init(&(*pool)->queue, queue_cap) != QUEUE_OK) {
                pthread_mutex_destroy(&(*pool)->mutex);
                free((*pool)->workers);
                free(*pool);
                return THREAD_POOL_QUEUE_ERROR;
        }

        (*pool)->shutdown = false;
        (*pool)->worker_count = workers;
        
        // initialize threads
        for (size_t w = 0; w < workers; w++) {
                if (pthread_create(&(*pool)->workers[w], NULL, worker, *pool) != 0) {
                        (*pool)->shutdown = true;
                        for (size_t i = 0; i < w; i++) {
                                job_queue_push((*pool)->queue, NULL);
                        }

                        for (size_t i = 0; i < w; i++) {
                                pthread_join((*pool)->workers[i], NULL);
                        }
                        
                        job_queue_destroy(&(*pool)->queue);
                        pthread_mutex_destroy(&(*pool)->mutex);
                        free((*pool)->workers);
                        free(*pool);
                        return THREAD_POOL_PTHREAD_ERROR;
                }
        }

        return THREAD_POOL_OK;
}


thread_pool_return_code_t thread_pool_submit(thread_pool_t *pool,
                                             job_func fn,
                                             void *args) {
        if (pool == NULL || fn == NULL) {
                return THREAD_POOL_NULL_ARGUMENT;
        }

        pthread_mutex_lock(&pool->mutex);

        if (pool->shutdown) {
                pthread_mutex_unlock(&pool->mutex);
                return THREAD_POOL_SHUTDOWNED;
        }


        job_t *j = NULL;
        if (job_init(&j, fn, args) != QUEUE_OK) {
                pthread_mutex_unlock(&pool->mutex);

                return THREAD_POOL_QUEUE_ERROR;
        }

        if (job_queue_push(pool->queue, j) != QUEUE_OK) {
                job_destroy(&j);
                pthread_mutex_unlock(&pool->mutex);
                return THREAD_POOL_QUEUE_ERROR;
        }
        pthread_mutex_unlock(&pool->mutex);
       
        return THREAD_POOL_OK;
}


void thread_pool_destroy(thread_pool_t **thread_pool) {
        if (thread_pool == NULL || *thread_pool == NULL) {
                return;
        }
                
        pthread_mutex_lock(&(*thread_pool)->mutex);

        (*thread_pool)->shutdown = true;
        
        pthread_mutex_unlock(&(*thread_pool)->mutex);
        
        // send NULL (sentinel) jobs for each worker
        for (size_t w = 0; w < (*thread_pool)->worker_count; w++) {
                job_queue_push((*thread_pool)->queue, NULL);
        }

        // join all threads
        for (size_t i = 0; i < (*thread_pool)->worker_count; i++) {
                pthread_join((*thread_pool)->workers[i], NULL);
        }
        free((*thread_pool)->workers);
        
        pthread_mutex_destroy(&(*thread_pool)->mutex);

        job_queue_destroy(&(*thread_pool)->queue);

        free(*thread_pool);

        *thread_pool = NULL;
}
