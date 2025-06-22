#include "thread-pool.h"


void ThreadPool::dispatcher() {
    while (true) {
        signalDispatcher->wait();
        if (shuttingDown) break;

        availableWorkers.wait();
        if (shuttingDown) break;

        function<void(void)> task;
        {
            lock_guard<mutex> lock(queueMutex);
            if (taskQueue.empty()) {
                availableWorkers.signal();
                continue;
            }
            task = taskQueue.front();
            taskQueue.pop();
        }

        for (size_t i = 0; i < workers.size(); ++i) {
            unique_lock<mutex> lock(workers[i].m);
            if (workers[i].available) {
                workers[i].available = false;
                workers[i].thunk = task;
                workers[i].ready->signal();
                break;
            }
        }
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        workers[id].ready->wait();
        if (shuttingDown) break;

        function<void(void)> taskCopy;
        {
            lock_guard<mutex> lock(workers[id].m);
            taskCopy = workers[id].thunk;
        }

        taskCopy();

        {
            lock_guard<mutex> lock(workers[id].m);
            workers[id].available = true;
        }
        availableWorkers.signal();

        {
            lock_guard<mutex> lock(wait_mutex);
            tasksInFlight--;
            if (tasksInFlight == 0)
                cv_wait.notify_all();
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(wait_mutex);
    cv_wait.wait(lock, [this] { return tasksInFlight == 0; });
}

ThreadPool::~ThreadPool() {
    wait();
    shuttingDown = true;

    signalDispatcher->signal();
    for (auto& w : workers) {
        w.ready->signal();
    }

    dispatcherThread.join();
    for (auto& w : workers) {
        if (w.t.joinable()) w.t.join();
        delete w.ready;
    }
    delete signalDispatcher;
}

ThreadPool::ThreadPool(size_t numThreads)
    : workers(numThreads),
      signalDispatcher(new Semaphore(0)),
      availableWorkers(numThreads),
      shuttingDown(false) {
    
    for (size_t i = 0; i < numThreads; ++i) {
        workers[i].ready = new Semaphore(0);
        workers[i].t = thread([this, i] { worker(i); });
    }
    dispatcherThread = thread([this] { dispatcher(); });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (!thunk) throw invalid_argument("Attempted to add an empty task.");
    if (shuttingDown) throw runtime_error("ThreadPool is no longer accepting tasks.");


    {
        lock_guard<mutex> lock(queueMutex);
        taskQueue.push(thunk);
    }

    {
        lock_guard<mutex> lock(wait_mutex);
        tasksInFlight++;
    }

    signalDispatcher->signal();
}