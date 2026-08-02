/*
 * This is a demo application that uses thread pool API.
 * This program receives a directory name from command line
 * and hashes all the regular files in the directory using SHA-256.
 */

#include "thread_pool.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <openssl/sha.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void *hash_file_sha256(void *args);

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
        if (argc != 4) {
                fprintf(stderr, "Usage: ./thread-pool-demo <dir_path> <worker_count> <queue_cap>\n");
                return EXIT_FAILURE;
        }
        thread_pool_t *pool = NULL;
        const char *dir_path = argv[1];
        const size_t WORKER_COUNT = (size_t)atoi(argv[2]);
        const size_t QUEUE_CAP = (size_t)atoi(argv[3]);

        // try to open the directory
        DIR *dir = opendir(dir_path);
        struct dirent *ent;
        if (dir == NULL) {
                perror("opendir");
                return EXIT_FAILURE;
        }
        
        if (thread_pool_init(&pool, WORKER_COUNT, QUEUE_CAP)
                        != THREAD_POOL_OK) {
                fprintf(stderr, "thread_pool_init: Thread pool couldn't created\n");
                closedir(dir);
                return EXIT_FAILURE;
        }

        while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_REG) {
                        char *file_path = malloc(sizeof(char) * PATH_MAX);
                        if (file_path == NULL) {
                                fprintf(stderr, "malloc: Memory couldn't allocated for file path\n");
                                errno = 0;
                                continue;
                        }
                        int n = snprintf(file_path,
                                         PATH_MAX,
                                         "%s/%s",
                                         dir_path,
                                         ent->d_name);
                        
                        if (n < 0 || (size_t)n >= PATH_MAX) {
                                fprintf(stderr, "snprintf: File path too long\n");
                                free(file_path);
                                errno = 0;
                                continue;
                        }

                        if (thread_pool_submit(pool, hash_file_sha256, file_path)
                                        != THREAD_POOL_OK) {
                                fprintf(stderr, "thread_pool_submit: Job couldn't be submitted\n");
                                free(file_path);
                        } 
                }
        }
        if (errno != 0) {
                perror("readdir");
                closedir(dir);
                thread_pool_destroy(&pool);
                return EXIT_FAILURE;
        }
        
        if (closedir(dir) == -1) {
                perror("closedir");
                thread_pool_destroy(&pool);
                return EXIT_FAILURE;
        }
        
        thread_pool_destroy(&pool);

        return EXIT_SUCCESS;
}

void *hash_file_sha256(void *args) {
        char *file_path = (char *)args;

        FILE *file = fopen(file_path, "rb");
        if (file == NULL) {
                perror("fopen");
                free(file_path);
                return NULL;
        }
                
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        
        size_t bytes_read;
        char buffer[BUFFER_SIZE];
        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                SHA256_Update(&ctx, buffer, bytes_read);
        }
        if (ferror(file)) {
                perror("fread");
                free(file_path);
                return NULL;
        }

        SHA256_Final(hash, &ctx);
        
        if (fclose(file) == EOF) {
                free(file_path);
                perror("fclose");
                return NULL;
        }

        // print hash value
        pthread_mutex_lock(&mut);

        printf("%s: ", file_path);
        for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                printf("%02x", hash[i]);
        }
        printf("\n");

        pthread_mutex_unlock(&mut);
        

        free(file_path);

        return NULL;
}
