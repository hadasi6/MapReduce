#ifndef MAPREDUCECLIENT_H
#define MAPREDUCECLIENT_H

#include <vector>
#include <utility>

// ======================[ Key/Value Base Classes ]==================

/**
 * @brief Base class for input key.
 */
class K1 {
public:
    virtual ~K1() {}
    virtual bool operator<(const K1& other) const = 0;
};

/**
 * @brief Base class for input value.
 */
class V1 {
public:
    virtual ~V1() {}
};

/**
 * @brief Base class for intermediate key.
 */
class K2 {
public:
    virtual ~K2() {}
    virtual bool operator<(const K2& other) const = 0;
};

/**
 * @brief Base class for intermediate value.
 */
class V2 {
public:
    virtual ~V2() {}
};

/**
 * @brief Base class for output key.
 */
class K3 {
public:
    virtual ~K3() {}
    virtual bool operator<(const K3& other) const = 0;
};

/**
 * @brief Base class for output value.
 */
class V3 {
public:
    virtual ~V3() {}
};

// ======================[ Type Definitions ]========================

typedef std::pair<K1*, V1*> InputPair;
typedef std::pair<K2*, V2*> IntermediatePair;
typedef std::pair<K3*, V3*> OutputPair;

typedef std::vector<InputPair> InputVec;
typedef std::vector<IntermediatePair> IntermediateVec;
typedef std::vector<OutputPair> OutputVec;

// ======================[ MapReduceClient Interface ]===============

/**
 * @brief Abstract base class for user MapReduce implementation.
 */
class MapReduceClient {
public:
    /**
     * @brief Map function: emits (K2, V2) pairs via emit2.
     */
    virtual void map(const K1* key, const V1* value, void* context) const = 0;

    /**
     * @brief Reduce function: emits (K3, V3) pairs via emit3.
     */
    virtual void reduce(const IntermediateVec* pairs, void* context) const = 0;
};

#endif // MAPREDUCECLIENT_H
