#include "Scheduler.h"

#include <iostream>

using namespace std::chrono_literals;

namespace sched
{

Scheduler::Scheduler()
{
    _running = true;
    _execution_thread = std::thread(&Scheduler::execution_thread, this);
}

Scheduler::~Scheduler()
{
    {
        std::scoped_lock lock{_executionMtx};
        _running = false;
    }
    _exectution_cv.notify_one();
    _execution_thread.join();
    _call_futures.clear(); // blocks until all started executions end
}

void Scheduler::execution_thread()
{
    while (_running)
    {
        if (_schedule.empty())
        {
            std::unique_lock lock{_executionMtx};
            _exectution_cv.wait(lock, [&](){ return _new_entry_available || !_running; });
            _new_entry_available = false;
            continue;
        }
        std::unique_lock lock{_executionMtx};
        auto no_timeout = _exectution_cv.wait_until(lock, _schedule.front().next_call, [&](){ return _new_entry_available || !_running; });
        if (no_timeout)
        {
            _new_entry_available = false;
            continue;
        }
        auto entry = _schedule.begin();
        _call_futures.emplace_back(std::async(std::launch::async,
            [&]()
            {
                entry->callback();
            }
        ));
        if (entry->repeated)
        {
            entry->next_call += entry->interval.value();
            if (_schedule.size() == 1) continue;
            auto it = find_insert_location(entry + 1, _schedule.end(), *entry);
            std::rotate(_schedule.begin(), _schedule.begin() + 1, it);
        }
        else
        {
            _schedule.erase(entry);
        }
    }
}

Scheduler::iter_t Scheduler::find_insert_location(iter_t begin, iter_t end, const Entry& entry)
{
    return std::lower_bound(begin, end, entry, 
        [&](const Entry& entry, const Entry& current_entry) -> bool 
        { 
            return entry.next_call < current_entry.next_call;
        }
    );
}

void Scheduler::insert(Entry&& entry)
{   
    {
        std::scoped_lock lock{_executionMtx};
        auto it = find_insert_location(_schedule.begin(), _schedule.end(), entry);
        _schedule.insert(it, std::forward<Entry>(entry));
        _new_entry_available = true;
    }
    _exectution_cv.notify_one();
}

}