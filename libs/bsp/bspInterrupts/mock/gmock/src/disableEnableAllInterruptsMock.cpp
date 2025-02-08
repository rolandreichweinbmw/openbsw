// Copyright 2024 Accenture.

#include "interrupts/disableEnableAllInterruptsMock.h"

namespace interrupts
{
uint32_t DisableEnableAllInterruptsMock::disableAllInterruptsCount = 0U;
uint32_t DisableEnableAllInterruptsMock::enableAllInterruptsCount  = 0U;
} // namespace interrupts

extern "C"
{
void disableAllInterrupts(void)
{
    if (::interrupts::DisableEnableAllInterruptsMockSingleton::is_valid())
    {
        ::interrupts::DisableEnableAllInterruptsMockSingleton::instance().disableAllInterrupts();
    }
    else
    {
        ++::interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount;
    }
}

void enableAllInterrupts(void)
{
    if (::interrupts::DisableEnableAllInterruptsMockSingleton::is_valid())
    {
        ::interrupts::DisableEnableAllInterruptsMockSingleton::instance().enableAllInterrupts();
    }
    else
    {
        ++::interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount;
    }
}

bool areInterruptsDisabled(void)
{
    if (::interrupts::DisableEnableAllInterruptsMockSingleton::is_valid())
    {
        return ::interrupts::DisableEnableAllInterruptsMockSingleton::instance()
            .areInterruptsDisabled();
    }
    return (
        interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount
        != interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount);
}

bool areInterruptsEnabled(void)
{
    if (::interrupts::DisableEnableAllInterruptsMockSingleton::is_valid())
    {
        return ::interrupts::DisableEnableAllInterruptsMockSingleton::instance()
            .areInterruptsEnabled();
    }
    return (
        interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount
        == interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount);
}

} // extern "C" {
