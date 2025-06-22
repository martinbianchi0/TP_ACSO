#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Semaphore.h"

using namespace std;

struct Worker {
    thread t;
    function<void(void)> thunk;
    bool available = true;
    Semaphore* ready = nullptr;
    mutex m;
};

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    void schedule(const function<void(void)>& thunk);
    void wait();
    ~ThreadPool();

private:
    void dispatcher();
    void worker(int id);

    vector<Worker> workers;
    thread dispatcherThread;

    queue<function<void(void)>> taskQueue;
    mutex queueMutex;

    Semaphore* signalDispatcher;
    Semaphore availableWorkers;

    condition_variable cv_wait;
    mutex wait_mutex;
    size_t tasksInFlight = 0;

    atomic<bool> shuttingDown;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};

#endif
