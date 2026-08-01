# Blocking Concurrent Job Queue

A thread-safe, blocking FIFO job queue implementation written in C using POSIX threads (`pthread`).

This library provides a generic job abstraction and a blocking concurrent queue that can safely be shared between multiple producer and consumer threads.

## Features

- Thread-safe FIFO queue
- Multiple producers and multiple consumers
- Blocking `push()` when the queue is full
- Blocking `pop()` when the queue is empty
- Generic jobs using function pointers
- Simple ownership model
- POSIX threads (`pthread`) based implementation
- Circular buffer for efficient memory usage

---

## Project Structure

```
.
├── include/
│   ├── queue.h
│   └── errcode.h
├── src/
│   └── queue.c
├── examples/
│   └── producer_consumer.c
├── Makefile
└── README.md
```

---

## Building

Compile the project using:

```bash
make
```

Remove generated files:

```bash
make clean
```

---

## API Overview

### Job

A job consists of

- a function
- an argument passed to that function

```c
typedef void *(*job_func)(void *);
```

Create a job:

```c
job_t *job;

job_init(&job, worker_function, argument);
```

Execute it:

```c
job_execute(job);
```

Destroy it:

```c
job_destroy(&job);
```

---

### Queue

Create a queue:

```c
job_queue_t *queue;

job_queue_init(&queue, 16);
```

Push a job:

```c
job_queue_push(queue, job);
```

Pop a job:

```c
job_t *job;

job_queue_pop(queue, &job);
```

Destroy the queue:

```c
job_queue_destroy(&queue);
```

---

## Example

A complete producer-consumer example is provided in

```
examples/producer_consumer.c
```

The example creates multiple producer and consumer threads.

- Producers create jobs and push them into the queue.
- Consumers pop jobs, execute them, and destroy them.

This demonstrates:

- concurrent producers
- concurrent consumers
- blocking behavior
- ownership transfer
- synchronization using condition variables

---

## Ownership Rules

### Jobs

After

```c
job_queue_push(queue, job);
```

the queue owns the job.

The caller must **not** destroy it.

After

```c
job_queue_pop(queue, &job);
```

ownership is transferred back to the caller.

The caller becomes responsible for calling

```c
job_destroy(&job);
```

---

### Queue

The queue object is owned by the library after successful initialization.

Destroy it using

```c
job_queue_destroy(&queue);
```

Do not call `free()` directly.

---

## Thread Safety

The queue is protected using

- `pthread_mutex_t`
- `pthread_cond_t`

Multiple producers and consumers may safely access the queue simultaneously.

Both operations are blocking:

- `job_queue_push()` blocks while the queue is full.
- `job_queue_pop()` blocks while the queue is empty.

---

## Notes

Before calling

```c
job_queue_destroy()
```

all producer and consumer threads **must have terminated**.

Destroying a queue while threads are blocked on it results in undefined behavior.

---

## Requirements

- C99
- POSIX Threads (`pthread`)
- GCC or Clang

---

## Future Improvements

Possible future extensions include:

- Timed push/pop operations
- Queue shutdown mechanism
- Thread pool implementation
- Unit tests
- CMake support

---

## License

This project is licensed under the MIT License.
