#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <memory>
#include "task.h"

class TaskPool {
public:
    TaskPool(size_t numThreads);

    ~TaskPool();

    void AddTask(std::shared_ptr<Task> task);

private:
    // This method should be called by every thread to take task from the pool
    // If there is no task available they sleep with the condition variable
    void Worker();

    std::vector<std::jthread> threads;    
    std::queue<std::shared_ptr<Task>> tasks;  
    std::mutex tasksMutex;                    
    std::condition_variable condVar;      // // Control variable used to wake or put a thread to sleep.
    std::atomic<bool> stop;               // Flag used to check if pool should stop or not
};
