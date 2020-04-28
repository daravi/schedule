#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace sched
{
class Scheduler
{
    using steady_time_t = std::chrono::steady_clock::time_point;
    
    struct Entry
    {
        unsigned int id;
        steady_time_t next_call;
        std::function<void()> callback{ nullptr };
        bool repeated{ false };
        std::optional<std::chrono::microseconds> interval{ std::nullopt };
    };
    
public:
    Scheduler();
    ~Scheduler();
    
    template <
        class Fn,
        class... Args,
        typename = std::enable_if_t<
            std::is_invocable<Fn, Args...>::value 
            && std::is_same<std::invoke_result_t<Fn, Args...>, void>::value
        >
    >
    unsigned int registerCall(const std::chrono::steady_clock::time_point& start, Fn&& f, Args&&... args)
    {
        insert(Entry{
            ++_last_id,
            start,
            std::bind(std::forward<Fn>(f), std::forward<Args>(args)...),
            false,
            std::nullopt
        });
        return _last_id;
    }
    
    template <
        class Rep,
        class Period,
        class Fn,
        class... Args,
        typename = std::enable_if_t<
            std::is_invocable<Fn, Args...>::value 
            && std::is_same<std::invoke_result_t<Fn, Args...>, void>::value
        >
    >
    unsigned int registerRepeatedCall(const std::chrono::steady_clock::time_point& start, const std::chrono::duration<Rep, Period>& dur, Fn&& f, Args&&... args)
    {
        insert(Entry{
            ++_last_id,
            start,
            std::bind(std::forward<Fn>(f), std::forward<Args>(args)...),
            true,
            std::chrono::duration_cast<std::chrono::microseconds>(dur)
        });
        return _last_id;
    }
    
    template <
        class Rep,
        class Period,
        class Fn,
        class... Args,
        typename = std::enable_if_t<
            std::is_invocable<Fn, Args...>::value 
            && std::is_same<std::invoke_result_t<Fn, Args...>, void>::value
        >
    >
    unsigned int registerRepeatedCall(const std::chrono::duration<Rep, Period>& dur, Fn&& f, Args&&... args)
    {
        return registerRepeatedCall(std::chrono::steady_clock::now(), dur, std::forward<Fn>(f), std::forward<Args>(args)...);
    }
    
    void unregisterCall(unsigned int call_id);
    
private:
    void execution_thread();
    void insert(Entry&& entry);
    
    using iter_t = std::vector<Entry>::iterator;
    static iter_t find_insert_location(iter_t begin, iter_t end, const Entry& entry);
    
    unsigned int _last_id{ 0 };
    std::vector<Entry> _schedule;
    
    std::thread _execution_thread;
    std::vector<std::future<void>> _call_futures; /* TODO PUYA: Do we need std::future or std::thread is enough? */
    std::condition_variable _exectution_cv;
    std::mutex _executionMtx;
    bool _running{ false };
    bool _schedule_updated{ false };
};
}