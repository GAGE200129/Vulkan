#include <pch.hpp>
#include "ThreadPool.hpp"

namespace gage::utils
{
    ThreadPool::ThreadPool(uint32_t num_threads)
    {
        for (uint32_t i = 0; i < num_threads; i++)
        {
            ThreadData data{
                .thread = nullptr,
                .mutex = std::make_unique<std::mutex>(),
                .job = nullptr,
                .cv = std::make_unique<std::condition_variable>(),
            };
            thread_datas.emplace_back(std::move(data));
        }

        // Launch threads
        for (uint32_t i = 0; i < num_threads; i++)
        {
            thread_datas.at(i).thread = std::make_unique<std::thread>(&ThreadPool::loop, this, i);
        }
    }
    ThreadPool::~ThreadPool()
    {
        terminate = true;
        for (auto &thread_data : thread_datas)
        {
            thread_data.cv->notify_all();
            thread_data.thread->join();
        }
        thread_datas.clear();
    }

    void ThreadPool::set_job(uint32_t thread_index, const std::function<void()> &job)
    {
        std::unique_lock<std::mutex> lock(*thread_datas.at(thread_index).mutex);
        thread_datas.at(thread_index).job = std::make_unique<std::function<void()>>(job);
        thread_datas.at(thread_index).cv->notify_one();
    }

    bool ThreadPool::wait()
    {

        std::unique_lock<std::mutex> lock(main_mutex);      
        main_cv.wait(lock, [&]() { 
            for (const auto &thread_data : thread_datas)
            {
                if(thread_data.job)
                    return false;
            }

            return true;
        });
        
        return false;
    }


    void ThreadPool::loop(uint32_t thread_index)
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(*thread_datas.at(thread_index).mutex);
            thread_datas.at(thread_index).cv->wait(lock, [this, &thread_index]
                                                   { return thread_datas.at(thread_index).job || terminate; });

            if (terminate)
            {
                return;
            }
            std::unique_ptr<std::function<void()>> *job = &thread_datas.at(thread_index).job;
            (**job)();
            job->reset();

            main_cv.notify_all();
        }
    }
}