#include <iostream>
#include <mutex>
#include <vector>
#include <thread>
#include <random>
#include <condition_variable>
#include "LockGuard.h"

std::mutex gMutex;
std::condition_variable gCv;
int gThreadIndex = 0;
const int gThreadCount = 3;

class RandomEngine
{
    std::random_device random_device_;
    std::mt19937 random_engine_;
    std::uniform_int_distribution<int> distribution_;
    static std::mutex mutex_;

public:
    static RandomEngine& instance()
    {
        Challenge::lock_guard<std::mutex> lock(mutex_);

        static RandomEngine r;
        return r;
    }

    int randomNumber()
    {
        Challenge::lock_guard<std::mutex> lock(mutex_);

        return distribution_(random_engine_);
    }

    RandomEngine(RandomEngine const&) = delete;
    void operator=(RandomEngine const&) = delete;

    RandomEngine(RandomEngine&&) = delete;
    void operator=(RandomEngine&&) = delete;

private:
    RandomEngine()
    :   random_engine_(random_device_()),
        distribution_(1,5)
    {
    }
};

std::mutex RandomEngine::mutex_;

namespace
{
    void logThreadStatus(int threadIndex, const char* statusStr)
    {
        std::cout << "thread " << threadIndex << ": " << statusStr << std::endl;
    }

    void signalThread(int index)
    {
        {
            std::lock_guard<std::mutex> lock(gMutex);
            gThreadIndex = index;
        }
        gCv.notify_all();
    }

    void threadFunction(int threadIndex)
    {
        std::cout << "thread" << threadIndex << ": starting, waiting." << std::endl;
        while (true)
        {
            int newThreadIndex = threadIndex;

            {
                std::unique_lock<std::mutex> lock(gMutex);
                gCv.wait(lock, [threadIndex]
                { 
                    return (threadIndex == gThreadIndex); 
                });

                logThreadStatus(threadIndex, "signal received, doing work ....");
                // sleep emulates some time consuming calculation
                std::this_thread::sleep_for(std::chrono::seconds(RandomEngine::instance().randomNumber()));
                logThreadStatus(threadIndex, "done with work, signal next thread");
                if (threadIndex == gThreadCount)
                    newThreadIndex = 0;
            }

            signalThread(newThreadIndex + 1);
        }
    }
}

int main()
{
    std::vector<std::thread> threads;
    std::cout << "main: starting all threads" << std::endl;
    for (int i = 1; i <= gThreadCount; i++)
    {
        threads.push_back(std::move(std::thread([i]()
        {
            threadFunction(i);
        })));
    }

    // let to signal wait before the signalization starts
    std::this_thread::sleep_for(std::chrono::seconds(1));

    signalThread(gThreadIndex + 1);
    for (auto& t : threads)
    {
        t.join();
    }


    return 1;
}
