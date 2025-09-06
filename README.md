# MapReduce Framework (C++)

A modern, thread-safe C++ MapReduce framework for parallel data processing, designed according to academic and industry standards.

## Overview
This project implements a generic, multi-threaded MapReduce framework in C++. The framework separates the **client** (user-defined map/reduce logic) from the **framework** (thread management, synchronization, and job orchestration), allowing easy extension for new data processing tasks.

## Features
- Multi-threaded execution using C++11 threads
- Efficient work distribution using atomic variables
- Custom barrier implementation for phase synchronization
- Thread-safe output collection with mutex protection
- Lock-free progress tracking and job state management with `std::atomic`
- Clean, modular, and well-documented codebase
- Simple API for custom client logic (see `examples/SampleClient.cpp`)
- Asynchronous job execution and progress querying

## Technical Highlights
- **Parallelism:** Map, sort, shuffle, and reduce phases are distributed across multiple threads for high performance.
- **Atomic Operations:** All progress and state tracking is done with atomic variables for correctness and speed.
- **Barrier Synchronization:** Ensures all threads complete each phase before moving to the next.
- **Shuffle Phase:** Efficient grouping of intermediate results, handled by a single thread for correctness.
- **Thread Safety:** All shared resources are protected; output vector is mutex-guarded.
- **Extensibility:** Users can easily implement their own key/value types and map/reduce logic.

## API Overview
- `startMapReduceJob`: Start a MapReduce job asynchronously.
- `waitForJob`: Wait for a job to finish (safe to call multiple times).
- `getJobState`: Query the current stage and progress of a job.
- `closeJobHandle`: Release all resources after job completion.
- `emit2`, `emit3`: Used by client code to emit intermediate and output pairs.

See `src/MapReduceFramework.h` and `src/MapReduceClient.h` for full API details.

## Example Usage
```cpp
#include "src/MapReduceFramework.h"
// ... define your MapReduceClient ...
JobHandle job = startMapReduceJob(client, inputVec, outputVec, 4);
waitForJob(job);
closeJobHandle(job);
```
See `examples/SampleClient.cpp` for a full working example (character frequency count).

## Project Structure
```
src/        # Framework source and headers
examples/   # Example client(s)
Makefile    # Build instructions
README.md   # Project documentation (this file)
```

## Build

```sh
make
```

## Run Example

```sh
make example
./sample_client
```

## Design Notes
- The framework is fully thread-safe and supports concurrent job execution.
- All synchronization is done with C++11 primitives (std::thread, std::mutex, std::atomic).
- No memory leaks: all allocations are managed and released.
- System errors are reported and terminate the process as required.
- The framework does not contain a main function or print output (except error reporting).
