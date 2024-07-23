#pragma once

//https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

namespace gage::utils
{
    class ThreadPool
    {
        struct ThreadData
        {
            std::unique_ptr<std::thread> thread;
            std::unique_ptr<std::mutex> mutex;
            std::unique_ptr<std::function<void()>> job;
            std::unique_ptr<std::condition_variable> cv;
        };
    public:
        ThreadPool(uint32_t num_threads);
        ~ThreadPool();

        void set_job(uint32_t thread_index, const std::function<void()>& job);
        bool wait(); // Wait for all thread to finish
    private:
        void loop(uint32_t thread_index);
    private:
        bool terminate{};

        std::mutex main_mutex;
        std::condition_variable main_cv;
        std::vector<ThreadData> thread_datas;
    };
}