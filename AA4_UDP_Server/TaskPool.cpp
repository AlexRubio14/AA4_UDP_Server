#include "TaskPool.h"

TaskPool::TaskPool(size_t numThreads)
    : stop(false) 
{
    // Create the jthreads and set the worker method
    for (size_t i = 0; i < numThreads; ++i) {
        threads.push_back(std::jthread(&TaskPool::Worker, this));
    }
}

TaskPool::~TaskPool()
{
    stop = true;  // After delete the taskPool say to every thread that should stop
    condVar.notify_all();  // Wake every thread to end their job
}

void TaskPool::AddTask(std::shared_ptr<Task> task)
{
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        tasks.push(task);
    }
    condVar.notify_one();  // Wake the next available jthread in the queue of conditionalVar
}

void TaskPool::Worker()
{
    while (!stop) {
        std::shared_ptr<Task> task;

        {
            // Check if taskPool is avaliable
            std::unique_lock<std::mutex> lock(tasksMutex);
            condVar.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) 
                return;

            // If there are tasks in the pool get the first task of the pool
            task = tasks.front();
            tasks.pop();
        }

        task->Execute();  // Ejecutar la tarea
    }
}
