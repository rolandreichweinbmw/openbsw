// Copyright 2024 Accenture.

#include "systems/DoCanSystem.h"

#include "systems/ICanSystem.h"
#include "transport/ITransportSystem.h"

#include <app/appConfig.h>
#include <bsp/timer/SystemTimer.h>
#include <docan/common/DoCanLogger.h>
#include <docan/datalink/DoCanFrameCodecConfigPresets.h>
#include <etl/delegate.h>
#include <etl/span.h>

namespace
{
uint32_t const TIMEOUT_DOCAN_SYSTEM   = 10U;
size_t const TICK_DELTA_TICKS         = 2U;   // Tick delta
size_t const TICK_100MS               = 100U; // 100us ticks
uint16_t const ALLOCATE_TIMEOUT       = 1000U;
uint16_t const RX_TIMEOUT             = 1000U;
uint16_t const TX_CALLBACK_TIMEOUT    = 1000U;
uint16_t const FLOW_CONTROL_TIMEOUT   = 1000U;
uint8_t const ALLOCATE_RETRY_COUNT    = 15U;
uint8_t const FLOW_CONTROL_WAIT_COUNT = 15U;
uint16_t const MIN_SEPARATION_TIME    = 2U;
uint8_t const BLOCK_SIZE              = 15U;

uint32_t systemUs() { return getSystemTimeUs32Bit(); }

} // namespace

namespace docan
{

DoCanSystem::AddressingFilterType::AddressEntryType DoCanSystem::_addresses[]
    = {{0x02A, 0x0F0U, 0x0F0U, LOGICAL_ADDRESS, 0, 0}};

DoCanSystem::DoCanSystem(
    ::transport::ITransportSystem& transportSystem,
    ::can::ICanSystem& canSystem,
    ::async::ContextType asyncContext)
: _context(asyncContext)
, _cyclicTimeout()
, _canSystem(canSystem)
, _transportSystem(transportSystem)
, _addressing()
, _frameSizeMapper()
, _classicCodec(::docan::DoCanFrameCodecConfigPresets::PADDED_CLASSIC, _frameSizeMapper)
, _classicAddressingFilter()
, _parameters(
      ::etl::delegate<decltype(systemUs)>::create<&systemUs>(),
      ALLOCATE_TIMEOUT,
      RX_TIMEOUT,
      TX_CALLBACK_TIMEOUT,
      FLOW_CONTROL_TIMEOUT,
      ALLOCATE_RETRY_COUNT,
      FLOW_CONTROL_WAIT_COUNT,
      MIN_SEPARATION_TIME,
      BLOCK_SIZE)
, _transportLayerConfig(_parameters)
, _physicalTransceivers()
, _transportLayers()
, _tickGenerator(asyncContext, _transportLayers)
, _codecs{&_classicCodec}
{
    setTransitionContext(asyncContext);
}

void DoCanSystem::initLayer()
{
    auto& transceiver = *_canSystem.getCanTransceiver(::busid::CAN_0);

    ::docan::DoCanPhysicalCanTransceiver<AddressingType>& doCanTransceiver
        = _physicalTransceivers.emplace_back(
            ::etl::ref(transceiver),
            ::etl::ref(_classicAddressingFilter),
            ::etl::ref(_classicAddressingFilter),
            ::etl::ref(_addressing));

    _transportLayers.emplace_back(
        ::busid::CAN_0,
        ::etl::ref(_context),
        ::etl::ref(_classicAddressingFilter),
        ::etl::ref(doCanTransceiver),
        ::etl::ref(_tickGenerator),
        ::etl::ref(_transportLayerConfig),
        ::util::logger::DOCAN);
}

void DoCanSystem::init()
{
    _classicAddressingFilter.init(
        ::etl::span<DoCanSystem::AddressingFilterType::AddressEntryType>(_addresses),
        ::etl::span<FrameCodecType const*>(_codecs));

    initLayer();

    transitionDone();
}

void DoCanSystem::run()
{
    for (auto& layer : _transportLayers.getTransportLayers())
    {
        _transportSystem.addTransportLayer(layer);
    }
    _transportLayers.init();

    ::async::scheduleAtFixedRate(
        _context, *this, _cyclicTimeout, TIMEOUT_DOCAN_SYSTEM, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

void DoCanSystem::shutdown()
{
    _cyclicTimeout.cancel();

    for (auto& layer : _transportLayers.getTransportLayers())
    {
        _transportSystem.removeTransportLayer(layer);
    }

    transitionDone();
}

void DoCanSystem::execute() { _transportLayers.cyclicTask(systemUs()); }

void DoCanSystem::TickGeneratorRunnableAdapter::scheduleTick()
{
    ::async::schedule(
        _context, *this, _tickTimeout, TICK_DELTA_TICKS * 100U, ::async::TimeUnit::MICROSECONDS);
}

DoCanSystem::TickGeneratorRunnableAdapter::TickGeneratorRunnableAdapter(
    ::async::ContextType const context, TransportLayers& layers)
: _layers(layers), _context(context)
{}

void DoCanSystem::TickGeneratorRunnableAdapter::cancelTimeout() { _tickTimeout.cancel(); }

void DoCanSystem::TickGeneratorRunnableAdapter::tickNeeded() { scheduleTick(); }

void DoCanSystem::TickGeneratorRunnableAdapter::execute()
{
    if (_layers.tick(systemUs()))
    {
        scheduleTick();
    }
}

} // namespace docan
