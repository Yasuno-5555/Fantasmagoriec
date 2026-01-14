#include "job_system.hpp"

namespace core {
    std::vector<std::thread> JobSystem::workers;
    std::queue<JobSystem::Job> JobSystem::jobs;
    std::mutex JobSystem::queue_mutex;
    std::condition_variable JobSystem::condition;
    std::atomic<bool> JobSystem::stop = false;

    void JobSystem::Init(int thread_count) {
        if (!workers.empty()) return; // Already init
        
        stop = false;
        if (thread_count <= 0) {
            thread_count = std::thread::hardware_concurrency();
            if (thread_count == 0) thread_count = 4;
            // Leave one core for Main Thread usually? 
            // thread_count = std::max(1, thread_count - 1);
        }

        for(int i=0; i<thread_count; ++i) {
            workers.emplace_back([]{
                while(true) {
                    Job job;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, []{ return stop || !jobs.empty(); });
                        if(stop && jobs.empty()) return;
                        job = std::move(jobs.front());
                        jobs.pop();
                    }
                    job();
                }
            });
        }
    }

    void JobSystem::Dispatch(Job job) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            jobs.push(std::move(job));
        }
        condition.notify_one();
    }

    void JobSystem::Shutdown() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers) {
            if(worker.joinable()) worker.join();
        }
        workers.clear();
    }
}
