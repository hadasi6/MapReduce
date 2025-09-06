# MapReduce Framework (C++)

![C++11](https://img.shields.io/badge/C%2B%2B-11-blue)
![Threads](https://img.shields.io/badge/Concurrency-Multi--Threaded-green)
![Status](https://img.shields.io/badge/Build-Makefile-success)

A modern, thread-safe C++ MapReduce framework for **parallel data processing**.  
The design cleanly separates user-defined map/reduce logic (client) from the execution engine (framework) that manages threads, synchronization, and job orchestration.

---

## Overview

The framework runs the classic Map → (Sort) → Shuffle → Reduce pipeline in parallel where appropriate, while ensuring correctness and clear resource ownership.

Flow (conceptual):

```
+------------------+     +---------+     +-----------+     +---------+
|    Input Data    | --> |  MAP    | --> |  SHUFFLE  | --> |  REDUCE |
+------------------+     +---------+     +-----------+     +---------+
       |              (multi-threaded)     (single-thread)   (multi-threaded)


```

Time/threads (example with 4 threads):

```
Thread 0:  MAP ---> SORT ----\
Thread 1:  MAP ---> SORT -----\                           ---> REDUCE
Thread 2:  MAP ---> SORT ------>  [ Barrier ] --> SHUFFLE ---> REDUCE
Thread 3:  MAP ---> SORT -----/                           ---> REDUCE
                                                              
````

---

## Technical Highlights

- **Concurrency:** Parallel execution of Map/Sort/Reduce using `std::thread`.
- **Synchronization:** Custom barrier; progress/state tracking with `std::atomic`.
- **Thread Safety:** Shared output guarded by `std::mutex`; lock-free job state.
- **Asynchronous Jobs:** Start, monitor, and close jobs independently.
- **Extensibility:** Pluggable key/value types and client-side map/reduce logic.
- **Resource Safety:** No leaks; all allocations are owned and released deterministically.  
- **Error Handling:** Required system errors terminate the process as specified.

---

## API Overview
- `startMapReduceJob`: Start a MapReduce job asynchronously.
- `waitForJob`: Wait for a job to finish (safe to call multiple times).
- `getJobState`: Query the current stage and progress of a job.
- `closeJobHandle`: Release all resources after job completion.
- `emit2`, `emit3`: Used by client code to emit intermediate and output pairs.

See [`src/MapReduceFramework.h`](src/MapReduceFramework.h) and
[`src/MapReduceClient.h`](src/MapReduceClient.h) for full details.

---

## Example

```cpp
#include "src/MapReduceFramework.h"

// Define a MapReduceClient (see examples/SampleClient.cpp)
JobHandle job = startMapReduceJob(client, inputVec, outputVec, /*threads=*/4);
waitForJob(job);
closeJobHandle(job);
```

A working demo (character frequency) is provided in
[`examples/SampleClient.cpp`](examples/SampleClient.cpp).

---

## Project Structure

```
src/        # Framework sources and headers
examples/   # Example clients
Makefile    # Build instructions
README.md   # This document
```

---

## Design Notes

* Uses only standard C++11 primitives: `std::thread`, `std::mutex`, `std::atomic`.
* The shuffle phase is single-threaded by design for deterministic grouping.
* The framework contains no `main()` and prints no output except mandated error messages.

---

## Skills Demonstrated

* C++11 concurrency (threads, atomics, mutexes, barriers)
* Parallel programming & phase orchestration
* Lock-free state/progress tracking
* Performance-aware design and explicit resource ownership

```
