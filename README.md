# Thread Pool in C

A lightweight **fixed-size thread pool** implementation written in **C99** using **POSIX Threads (pthreads)**.

This project provides a reusable thread pool API that allows applications to execute tasks asynchronously using a fixed number of worker threads. Internally, it uses a **blocking concurrent queue** implemented as a separate library, demonstrating modular software design and code reuse.

---

## Features

- Fixed-size thread pool
- Thread-safe task submission
- Blocking concurrent job queue
- Graceful shutdown
- Reusable C API
- Producer-consumer architecture
- Demo application included
- Written in ISO C99

---

## Project Structure

```text
.
├── demo
│   └── main.c                  # Demo application
├── external
│   └── job-queue               # Blocking concurrent queue library
│       ├── include
│       ├── src
│       └── README.md
├── include
│   └── thread_pool.h           # Public API
├── src
│   └── thread_pool.c           # Thread pool implementation
├── Makefile
└── README.md
```

---

## Architecture

The library consists of three main components:

- A fixed number of worker threads
- A blocking concurrent queue
- A simple API for submitting jobs

Applications submit tasks through the public API. Submitted jobs are stored inside the blocking queue, where idle worker threads wait until work becomes available.

```
                   thread_pool_submit()

                           │
                           ▼

                  +-------------------+
                  |  Blocking Queue   |
                  +-------------------+
                     ▲            ▲
                     │            │
             dequeue │            │ enqueue
                     │            │

      +--------------+------------+--------------+
      |              |            |              |
      ▼              ▼            ▼              ▼

   Worker 1      Worker 2     Worker 3      Worker N
```

---

## Dependency

This project **reuses** another library that I developed previously:

- **Blocking Concurrent Job Queue**

Instead of reimplementing the queue, the thread pool builds directly on top of that API. This demonstrates modular software development and library reuse.

The queue implementation is located under:

```text
external/job-queue/
```

---

## Public API

### Create a thread pool

```c
thread_pool_t *pool = NULL;

thread_pool_init(&pool, 4, 32);
```

- `4` → Number of worker threads
- `32` → Maximum queue capacity

---

### Submit a task

```c
thread_pool_submit(pool, my_task, my_data);
```

Every submitted task consists of

- a function pointer
- a pointer to user-defined arguments

---

### Destroy the pool

```c
thread_pool_destroy(&pool);
```

All worker threads are terminated gracefully before resources are released.

---

## Demo Application

The repository includes a demonstration program located in

```text
demo/main.c
```

The demo hashes every regular file inside a directory using **SHA-256**.

Workflow:

1. Open a directory.
2. Enumerate all regular files.
3. Submit one hashing task for each file.
4. Worker threads process files concurrently.
5. Print each file's SHA-256 hash.

Example:

```bash
./thread-pool-demo ./files
```

Example output:

```text
./files/a.txt: 3a7bd3...
./files/image.png: 5c18f2...
./files/report.pdf: a2f91e...
```

---

## Building

Compile the project:

```bash
make
```

Run the demo:

```bash
./thread-pool-demo <directory> <worker_count> <queue_capacity>
```

Example:

```bash
./thread-pool-demo ./test_files
```

---

## Requirements

- GCC
- POSIX Threads (`pthread`)
- OpenSSL (`libcrypto`)

Ubuntu/Debian:

```bash
sudo apt install build-essential libssl-dev
```

---

## Design Decisions

- Fixed-size worker pool
- Blocking producer-consumer queue
- Separate reusable queue library
- Thread-safe task submission
- Graceful shutdown using sentinel (poison pill) jobs
- Opaque public API (`thread_pool_t`)

---

## Future Improvements

Possible extensions include:

- Dynamic thread pool resizing
- Timed waits
- Task priorities
- Futures / promises
- Waiting for all submitted tasks
- Work stealing
- Task cancellation

---

## License

MIT License
