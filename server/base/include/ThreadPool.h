#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <vector>
#include <functional>
#include <condition_variable>
#include <string>

const int THREAD_NUMS = 16;
class ThreadPool
{

public:
    ThreadPool(int threads);
    ~ThreadPool();
    void add_task(std::function<void()> tmp);

private:
    void work();
    void stop();

    std::queue<std::function<void()>> tasks;
    std::mutex m_consumer;
    std::mutex m_productor;
    std::vector<std::thread> threads;
    std::condition_variable condition_consumer;

    bool STOP = false;
    bool has_stop = false;
};

#endif