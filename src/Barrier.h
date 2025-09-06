#ifndef BARRIER_H
#define BARRIER_H

#include <mutex>
#include <condition_variable>

/**
 * @brief Barrier for synchronizing a fixed number of threads.
 */
class Barrier {
public:
    explicit Barrier(int numThreads);
    ~Barrier() = default;

    /**
     * @brief Waits until all threads reach the barrier.
     */
    void barrier();

private:
    std::mutex mutex;
    std::condition_variable cv;
    int count;
    int generation;
    const int numThreads;
};

#endif // BARRIER_H
