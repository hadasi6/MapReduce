# MapReduce Framework (C++)

A modern, thread-safe C++ MapReduce framework for parallel data processing and demonstration of advanced concurrency techniques.

## Features
- Multi-threaded MapReduce engine using C++11 threads
- Uses `std::atomic` for lock-free progress tracking and safe state management
- Barrier synchronization for precise thread coordination
- Mutex-protected output collection for thread safety
- Clean, modular, and well-documented codebase
- Simple API for custom client logic (see `examples/SampleClient.cpp`)

## Technical Highlights
- **Parallelism:** Efficient use of multiple threads for map, shuffle, and reduce stages
- **Atomic Operations:** Progress and job state are tracked using `std::atomic` variables for high performance and correctness
- **Synchronization:** Custom barrier implementation ensures all threads advance together between stages
- **Extensibility:** Users can easily implement their own key/value types and map/reduce logic

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

## API Usage
- Implement your own `MapReduceClient` by inheriting from the abstract class in `src/MapReduceClient.h`.
- See `examples/SampleClient.cpp` for a full working example.
