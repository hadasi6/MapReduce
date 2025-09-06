#ifndef MAPREDUCEFRAMEWORK_H
#define MAPREDUCEFRAMEWORK_H

#include "MapReduceClient.h"

// ======================[ Type Definitions ]========================

typedef void* JobHandle;

enum stage_t {
    UNDEFINED_STAGE = 0,
    MAP_STAGE = 1,
    SHUFFLE_STAGE = 2,
    REDUCE_STAGE = 3
};

typedef struct {
    stage_t stage;
    float percentage;
} JobState;

// ======================[ API Functions ]===========================

/**
 * @brief Emits an intermediate (K2, V2) pair from the map function.
 */
void emit2(K2* key, V2* value, void* context);

/**
 * @brief Emits an output (K3, V3) pair from the reduce function.
 */
void emit3(K3* key, V3* value, void* context);

/**
 * @brief Starts a MapReduce job.
 */
JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec,
                            OutputVec& outputVec,
                            int multiThreadLevel);

/**
 * @brief Waits for the MapReduce job to finish.
 */
void waitForJob(JobHandle job);

/**
 * @brief Gets the current state of the MapReduce job.
 */
void getJobState(JobHandle job, JobState* state);

/**
 * @brief Releases all resources of the MapReduce job.
 */
void closeJobHandle(JobHandle job);

#endif // MAPREDUCEFRAMEWORK_H
