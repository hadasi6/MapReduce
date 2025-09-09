#include "MapReduceFramework.h"
#include "Barrier.h"
#include <thread>
#include <iostream>
#include <atomic>
#include <algorithm>
#include <queue>
#include <cstdlib> // For exit()

// ======================[ Constants & Macros ]======================

#define SYSTEM_ERROR_MSG(msg) std::cerr << "system error: " << msg << std::endl
#define EXIT_ON_ERROR(code) exit(code)
#define ERROR_EXIT_CODE 1

// ======================[ Internal Structs ]========================

// Forward declaration
struct JobContext;

/**
 * @brief Context for a single thread in the MapReduce job.
 */
struct ThreadContext {
    int threadID;
    JobContext* job;
    IntermediateVec intermediateVec;

    ThreadContext(int id, JobContext* jobContext)
        : threadID(id), job(jobContext), intermediateVec() {}
};

/**
 * @brief Context for the entire MapReduce job.
 */
struct JobContext {
    const MapReduceClient* client;         // Client's map/reduce implementation
    const InputVec* inputVec;              // Input vector for map phase
    OutputVec* outputVec;                  // Final output vector (from reduce)
    int threadCount;                       // Number of worker threads
    std::vector<std::thread> threads;      // Thread objects
    std::vector<ThreadContext> threadContexts; // Thread contexts
    std::atomic<int> vecIndex;             // Index for work distribution
    std::atomic<uint64_t> jobState;        // Encoded job state
    Barrier barrier;                       // Barrier for thread synchronization
    std::mutex outputMutex;                // Mutex for output vector
    std::vector<IntermediateVec> shuffledVecsQueue; // Shuffled intermediate groups
    bool calledWaitForJob;                 // Ensures waitForJob is called once

    JobContext(const MapReduceClient* client,
               const InputVec* inputVec,
               OutputVec* outputVec,
               int threadCount)
        : client(client),
          inputVec(inputVec),
          outputVec(outputVec),
          threadCount(threadCount),
          vecIndex(0),
          jobState(0),
          barrier(threadCount),
          calledWaitForJob(false)
    {
        threadContexts.reserve(threadCount);
        for (int i = 0; i < threadCount; ++i) {
            threadContexts.emplace_back(i, this);
        }
    }
};

// ======================[ Utility Functions ]=======================

/**
 * @brief Encodes the job state into a 64-bit integer.
 */
static uint64_t encodeJobState(int stage, uint64_t processed, uint64_t total) {
    return (static_cast<uint64_t>(stage) << 62) | (processed << 31) | total;
}

// ======================[ Shuffle Stage ]===========================

/**
 * @brief Performs the shuffle stage: groups all intermediate pairs by key.
 */
static void performShuffleStage(JobContext* job) {
    uint64_t totalPairs = 0;
    try {
        for (ThreadContext& tc : job->threadContexts) {
            totalPairs += tc.intermediateVec.size();
        }
        job->jobState.store(encodeJobState(SHUFFLE_STAGE, 0, totalPairs));
    } catch (const std::exception& e) {
        SYSTEM_ERROR_MSG("failed during shuffle stage: " << e.what());
        EXIT_ON_ERROR(ERROR_EXIT_CODE);
    }

    using PQElement = std::tuple<K2*, V2*, int>; // (key, value, threadIndex)
    auto comp = [](const PQElement& a, const PQElement& b) {
        return *(std::get<0>(b)) < *(std::get<0>(a)); // Min-heap by key
    };

    std::priority_queue<PQElement, std::vector<PQElement>, decltype(comp)> pq(comp);
    std::vector<size_t> indices(job->threadCount, 0);

    // Initialize heap with first element from each thread's intermediate vector
    for (int i = 0; i < job->threadCount; ++i) {
        if (!job->threadContexts[i].intermediateVec.empty()) {
            auto& vec = job->threadContexts[i].intermediateVec;
            pq.emplace(vec[0].first, vec[0].second, i);
            indices[i] = 1;
        }
    }

    while (!pq.empty()) {
        K2* currKey = std::get<0>(pq.top());
        IntermediateVec group;

        while (!pq.empty() &&
               !(*currKey < *std::get<0>(pq.top())) &&
               !(*std::get<0>(pq.top()) < *currKey)) {
            K2* key = std::get<0>(pq.top());
            V2* val = std::get<1>(pq.top());
            int i = std::get<2>(pq.top());
            pq.pop();
            group.emplace_back(key, val);

            if (indices[i] < job->threadContexts[i].intermediateVec.size()) {
                auto& vec = job->threadContexts[i].intermediateVec;
                pq.emplace(vec[indices[i]].first, vec[indices[i]].second, i);
                ++indices[i];
            }
            job->jobState.fetch_add(1ULL << 31); // Update processed count
        }
        job->shuffledVecsQueue.push_back(std::move(group));
    }
}

// ======================[ Thread Main Function ]====================

/**
 * @brief Main function executed by each worker thread.
 */
static void runMapReduceJob(ThreadContext* tc) {
    int threadId = tc->threadID;
    JobContext* job = tc->job;
    size_t index;

    // Map phase
    while (true) {
        index = job->vecIndex.fetch_add(1);
        if (index >= job->inputVec->size()) break;

        const K1* key = (*(job->inputVec))[index].first;
        const V1* value = (*(job->inputVec))[index].second;
        job->client->map(key, value, tc);

        job->jobState.fetch_add(1ULL << 31); // Update processed count
    }

    // Sort intermediate vector by key
    std::sort(tc->intermediateVec.begin(), tc->intermediateVec.end(),
              [](const IntermediatePair& a, const IntermediatePair& b) {
                  return *(a.first) < *(b.first);
              });

    job->barrier.barrier(); // Wait for all threads to finish map phase

    // Shuffle phase (only thread 0)
    if (threadId == 0) {
        performShuffleStage(job);
        job->jobState.store(encodeJobState(REDUCE_STAGE, 0, job->shuffledVecsQueue.size()));
        job->vecIndex.store(0); // Reset for reduce phase
    }
    job->barrier.barrier();

    // Reduce phase
    while (true) {
        index = job->vecIndex.fetch_add(1);
        if (index >= job->shuffledVecsQueue.size()) break;

        const IntermediateVec* vec = &(job->shuffledVecsQueue[index]);
        job->client->reduce(vec, tc);
        job->jobState.fetch_add(1ULL << 31);
    }
}

// ======================[ API Functions ]===========================

void waitForJob(JobHandle job) {
    JobContext* jobContext = static_cast<JobContext*>(job);
    if (!jobContext->calledWaitForJob) {
        try {
            for (auto& thread : jobContext->threads) {
                thread.join();
            }
        } catch (const std::system_error& e) {
            SYSTEM_ERROR_MSG("failed to join thread: " << e.what());
            EXIT_ON_ERROR(ERROR_EXIT_CODE);
        }
        jobContext->calledWaitForJob = true;
    }
}

JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec,
                            OutputVec& outputVec,
                            int multiThreadLevel) {
    JobContext* job = new JobContext(&client, &inputVec, &outputVec, multiThreadLevel);
    uint64_t initState = encodeJobState(MAP_STAGE, 0, job->inputVec->size());
    job->jobState.store(initState);

    for (int i = 0; i < multiThreadLevel; ++i) {
        try {
            job->threads.emplace_back(runMapReduceJob, &(job->threadContexts[i]));
        } catch (const std::system_error& e) {
            SYSTEM_ERROR_MSG("failed to create thread: " << e.what());
            EXIT_ON_ERROR(ERROR_EXIT_CODE);
        }
    }
    return job;
}

void emit2(K2* key, V2* value, void* context) {
    ThreadContext* tc = static_cast<ThreadContext*>(context);
    tc->intermediateVec.emplace_back(key, value);
}

void emit3(K3* key, V3* value, void* context) {
    ThreadContext* tc = static_cast<ThreadContext*>(context);
    JobContext* job = tc->job;
    try {
        std::unique_lock<std::mutex> lock(job->outputMutex);
        job->outputVec->emplace_back(key, value);
    } catch (const std::system_error& e) {
        SYSTEM_ERROR_MSG("failed to lock mutex or add to output vector: " << e.what());
        EXIT_ON_ERROR(ERROR_EXIT_CODE);
    }
}

void getJobState(JobHandle job, JobState* state) {
    JobContext* jobContext = static_cast<JobContext*>(job);
    uint64_t jobState;
    try {
        jobState = jobContext->jobState.load();
    } catch (const std::exception& e) {
        SYSTEM_ERROR_MSG("failed to load atomic job state: " << e.what());
        EXIT_ON_ERROR(ERROR_EXIT_CODE);
    }
    // Decode jobState
    state->stage = static_cast<stage_t>(jobState >> 62);
    uint64_t total = jobState & 0x7FFFFFFF;
    uint64_t processed = (jobState >> 31) & 0x7FFFFFFF;
    state->percentage = (total == 0) ? 100.0f : (100.0f * processed / total);
}

void closeJobHandle(JobHandle job) {
    JobContext* jobContext = static_cast<JobContext*>(job);
    waitForJob(job);
    delete jobContext;
}
