#pragma once
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace core {
    class JobSystem {
    public:
        using Job = std::function<void()>;
        
        static void Init(int thread_count = 0); // 0 = hardware concurrency
        static void Dispatch(Job job);
        static void Shutdown();
        
    private:
        static std::vector<std::thread> workers;
        static std::queue<Job> jobs;
        static std::mutex queue_mutex;
        static std::condition_variable condition;
        static std::atomic<bool> stop;
    };
}
