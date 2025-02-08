// Copyright 2024 Accenture.

#ifndef GUARD_5E72815E_E068_44C6_B357_404938726FD3
#define GUARD_5E72815E_E068_44C6_B357_404938726FD3

#include "timer/Timeout.h"

#include <cstdint>

namespace timer
{

/**
 * A template class that serves as collection of timeouts.
 * It manages and processes both cyclic and
 * one-shot timeouts, providing functionality to set timeouts,
 * check if a timer is active, and cancel the enqueued timeouts.
 *
 * \tparam LockGuard is the type that implements RAII-based lock for secure section.
 */
template<class LockGuard>
class Timer
{
public:
    using TimeoutList = ::etl::intrusive_forward_list<Timeout, ::etl::forward_link<0>>;

    /**
     * Called by the system to process the next elapsed timeout.
     * \param now Current system time
     * \return
     * - true if a timeout has been processed successfully and a next timeout should be processed
     * - false otherwise
     */
    bool processNextTimeout(uint32_t now);

    /**
     * Get the next timeout delta to set.
     * The caller has to make sure, that now is the current system time.
     * It shouldn't be reused from previous a processNextTimeout() call.
     * \param now Current system time
     * \param nextDelta reference to variable that receives next delta to set
     * \return
     * - true && nextDelta > 0 if the systems timeout should be rescheduled with nextTimeout
     * - true && nextDelta == 0 if an event in the system should be triggered immediately
     * - false otherwise
     */
    bool getNextDelta(uint32_t now, uint32_t& nextDelta) const;

    /**
     * Check whether a timer is active.
     * \param timeout Reference to Timeout
     * \return
     * - true if timer is event
     * - false otherwise
     */
    bool isActive(Timeout const& timeout) const;

    /**
     * Set a single shot timeout.
     * \param timeout Reference to Timeout
     * \param Delay relative value, indicating when the timer will be triggered
     * \param now Current system time
     * \return
     * - true if an event in the system should be triggered
     * - false otherwise
     */
    bool set(Timeout& timeout, uint32_t delay, uint32_t now);

    /**
     * Set a cyclic timeout.
     * \param timeout Reference to Timeout
     * \param period Time between cyclic timeouts
     * \param now Current system time
     * \return
     * - true if an event in the system should be triggered
     * - false otherwise
     */
    bool setCyclic(Timeout& timeout, uint32_t period, uint32_t now);

    /**
     * Cancel running timeout.
     * If the timeout is not scheduled (part of a list), cancel won't do anything.
     *
     * \param timeout Reference to Timeout
     */
    void cancel(Timeout& timeout);

private:
    void rescheduleCyclicTimeout(Timeout& timeout, uint32_t now);

    bool addTimeout(Timeout& timeout, uint32_t absoluteTimeout, uint32_t cycleTime, uint32_t now);

    static int32_t diff(uint32_t a, uint32_t const b);

    TimeoutList _timeoutList;
};

template<class LockGuard>
bool Timer<LockGuard>::processNextTimeout(uint32_t const now)
{
    Timeout* timeout    = nullptr;
    int32_t diffTimeout = 0;
    {
        LockGuard const scopedLock;
        if (!_timeoutList.empty())
        {
            diffTimeout = diff(_timeoutList.front()._time, now);
            if (diffTimeout <= 0)
            {
                timeout = &_timeoutList.front();
                _timeoutList.pop_front();
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    rescheduleCyclicTimeout(*timeout, now);
    timeout->expired();
    return diffTimeout == 0U;
}

template<class LockGuard>
bool Timer<LockGuard>::getNextDelta(uint32_t const now, uint32_t& nextDelta) const
{
    LockGuard const scopedLock;
    if (!_timeoutList.empty())
    {
        if (diff(_timeoutList.front()._time, now) < 0)
        {
            nextDelta = 0U;
        }
        else
        {
            nextDelta = _timeoutList.front()._time - now;
        }
        return true;
    }

    nextDelta = 0U;
    return false;
}

template<class LockGuard>
bool

Timer<LockGuard>::isActive( Timeout const & timeout) const
{
    return timeout.is_linked();
}

template<class LockGuard>
bool Timer<LockGuard>::set(Timeout& timeout, uint32_t const delay, uint32_t const now)
{
    return addTimeout(timeout, delay + now, 0U, now);
}

template<class LockGuard>
bool Timer<LockGuard>::setCyclic(Timeout& timeout, uint32_t const period, uint32_t const now)
{
    return addTimeout(timeout, period + now, period, now);
}

template<class LockGuard>
void Timer<LockGuard>::cancel(Timeout& timeout)
{
    if (timeout.is_linked())
    {
        LockGuard const scopedLock;
        _timeoutList.remove(timeout);
    }
}

template<class LockGuard>
void Timer<LockGuard>::rescheduleCyclicTimeout(Timeout& timeout, uint32_t const now)
{
    if (timeout._cycleTime > 0U)
    {
        (void)addTimeout(timeout, timeout._cycleTime + timeout._time, timeout._cycleTime, now);
    }
}

template<class LockGuard>
bool Timer<LockGuard>::addTimeout(
    Timeout& timeout, uint32_t const absoluteTimeout, uint32_t const cycleTime, uint32_t const now)
{
    timeout._time      = absoluteTimeout;
    timeout._cycleTime = cycleTime;

    LockGuard const lock;

    int32_t const timeoutDiff  = diff(timeout._time, now);
    TimeoutList::iterator prev = _timeoutList.before_begin();

    for (TimeoutList::iterator current = _timeoutList.begin(); current != _timeoutList.end();
         ++current)
    {
        int32_t const nextDiff = diff(current->_time, now);
        if (nextDiff > timeoutDiff)
        {
            break;
        }
        prev = current;
    }

    TimeoutList::iterator const insertPos = _timeoutList.insert_after(prev, timeout);
    return _timeoutList.begin() == insertPos; // will expire before all other
}

template<class LockGuard>
int32_t Timer<LockGuard>::diff(uint32_t const a, uint32_t const b)
{
    return static_cast<int32_t>(a - b);
}

} // namespace timer

#endif /* GUARD_5E72815E_E068_44C6_B357_404938726FD3 */
