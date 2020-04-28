#include "Scheduler.h"

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

void Scheduler::unregisterCall(unsigned int call_id)
{
    {
        std::unique_lock lock{_executionMtx};
        auto entry_it = std::find_if(_schedule.begin(), _schedule.end(),
        [&](const Entry& current_entry) -> bool
        {
            return current_entry.id == call_id;
        });
        
        _schedule.erase(entry_it);
        _schedule_updated = true;
    }
    _exectution_cv.notify_one();
}

void Scheduler::execution_thread()
{
    while (_running)
    {
        std::unique_lock lock{_executionMtx};
        if (_schedule.empty())
        {
            _exectution_cv.wait(lock, [&](){ return _schedule_updated || !_running; });
            _schedule_updated = false;
            continue;
        }
        
        auto next_call = _schedule.front().next_call;
        auto no_timeout = _exectution_cv.wait_until(lock, next_call, [&](){ return _schedule_updated || !_running; });
        
        if (no_timeout)
        {
            _schedule_updated = false;
            continue;
        }
        
        auto entry_it = _schedule.begin();
        
        _call_futures.emplace_back(std::async(std::launch::async,
            [entry{ *entry_it }]()
            {
                entry.callback();
            }
        ));
        
        if (entry_it->repeated)
        {
            entry_it->next_call += entry_it->interval.value();
            if (_schedule.size() == 1) continue;
            
            auto it = find_insert_location(entry_it + 1, _schedule.end(), *entry_it);
            std::rotate(_schedule.begin(), _schedule.begin() + 1, it);
        }
        else
        {
            _schedule.erase(entry_it);
        }
    }
}

Scheduler::iter_t Scheduler::find_insert_location(iter_t begin, iter_t end, const Entry& entry)
{
    return std::lower_bound(begin, end, entry,
        [](const Entry& current_entry, const Entry& entry) -> bool
        {
            return current_entry.next_call < entry.next_call;
        }
    );
}

void Scheduler::insert(Entry&& entry)
{
    {
        std::scoped_lock lock{_executionMtx};
        auto it = find_insert_location(_schedule.begin(), _schedule.end(), entry);
        _schedule.insert(it, std::forward<Entry>(entry));
        _schedule_updated = true;
    }
    _exectution_cv.notify_one();
}
}