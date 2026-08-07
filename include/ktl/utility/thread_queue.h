#pragma once

#if __cpp_lib_move_only_function >= 202110L

#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <vector>

namespace ktl
{
    class thread_queue
    {
    public:
        thread_queue() :
            m_Lock(),
            m_ThreadId(std::this_thread::get_id()),
            m_QueueFunctions() {}

        thread_queue(const thread_queue&) = delete;
        thread_queue(thread_queue&&) = delete;

        template<typename F>
        auto post(F func)
        {
            using R = decltype(func());

            std::promise<R> promise;
            std::future<R> future = promise.get_future();

            if (std::this_thread::get_id() == m_ThreadId)
            {
                if constexpr (std::is_void_v<R>)
                {
                    func();
                    promise.set_value();
                }
                else
                {
                    promise.set_value(func());
                }

                return future;
            }

            std::scoped_lock lock{ m_Lock };

            m_QueueFunctions.emplace_back([func, promise = std::move(promise)] mutable
                {
                    if constexpr (std::is_void_v<R>)
                    {
                        func();
                        promise.set_value();
                    }
                    else
                    {
                        promise.set_value(func());
                    }
                });

            return future;
        }

        bool run_once()
        {
            if (std::this_thread::get_id() != m_ThreadId)
                return false;

            std::scoped_lock lock{ m_Lock };

            if (m_QueueFunctions.empty())
                return false;

            for (size_t i = 0; i < m_QueueFunctions.size(); ++i)
            {
                m_QueueFunctions[i]();
            }

            m_QueueFunctions.clear();

            return true;
        }

    private:
        std::mutex m_Lock;
        std::thread::id m_ThreadId;
        std::vector<std::move_only_function<void()>> m_QueueFunctions;
    };
}

#endif