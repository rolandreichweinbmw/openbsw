// Copyright 2024 Accenture.

#include "systems/BspSystem.h"

#include <etl/alignment.h>
#include <etl/singleton.h>
#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif
#include "cache/cache.h"
#include "clock/clockConfig.h"
#include "commonDebug.h"
#include "interrupt_manager.h"
#include "lifecycle/StaticBsp.h"

#include <lifecycle/LifecycleManager.h>

extern void app_main();

extern "C"
{
void ExceptionHandler()
{
    while (true)
    {
        printf("ExceptionHandler :(");
    }
}

void boardInit()
{
    configurPll();
    cacheEnable();
}

void setupApplicationsIsr(void)
{
    // interrupts
    SYS_SetPriority(CAN0_ORed_0_15_MB_IRQn, 8);  // can0 buffer 0 - 15
    SYS_SetPriority(CAN0_ORed_16_31_MB_IRQn, 8); // can0 buffer 16 - 32

    SYS_EnableIRQ(CAN0_ORed_0_15_MB_IRQn);
    SYS_EnableIRQ(CAN0_ORed_16_31_MB_IRQn);

    ENABLE_INTERRUPTS();
}
} // extern "C"

namespace platform
{
StaticBsp staticBsp;

::etl::typed_storage<::systems::BspSystem> bspSystem;

#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::systems::CanSystem> canSystem;
#endif // PLATFORM_SUPPORT_CAN

/**
 * Callout from main application to give platform the chance to add a
 * ::lifecycle::ILifecycleComponent to the \p lifecycleManager at a given \p level.
 */
void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    if (level == 1U)
    {
        lifecycleManager.addComponent("bsp", bspSystem.emplace(TASK_BSP, staticBsp), level);
    }
    if (level == 2U)
    {
#ifdef PLATFORM_SUPPORT_CAN
        lifecycleManager.addComponent("can", canSystem.emplace(TASK_CAN, staticBsp), level);
#endif // PLATFORM_SUPPORT_CAN
    }
}
} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_CAN

int main()
{
    // StaticBsp::init() will disable the watchdog, this must be the first function call in main()!
    ::platform::staticBsp.init();
    printf("main(RCM::SRS 0x%lx)\r\n", *reinterpret_cast<uint32_t volatile*>(0x4007F008));
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
